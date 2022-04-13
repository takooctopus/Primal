	void create_a_root_signature() {
		// 三种参数依次举例
		D3D12_ROOT_PARAMETER1 params[3];
		{
			//param 0:2 cosntants
			auto& param = params[0];
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			D3D12_ROOT_CONSTANTS consts{};
			consts.Num32BitValues = 2;
			consts.ShaderRegister = 0; // b0
			consts.RegisterSpace = 0;
			param.Constants = consts;
			param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		}
		{
			//param 1:2 Constant Buffer View (descriptor)
			auto& param = params[1];
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			D3D12_ROOT_DESCRIPTOR1 root_desc{};
			root_desc.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
			root_desc.ShaderRegister = 1;
			root_desc.RegisterSpace = 0;
			param.Descriptor = root_desc;
			param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		}
		{
			//param 2:2 descriptor table
			auto& param = params[2];
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			D3D12_ROOT_DESCRIPTOR_TABLE1 table{};
			table.NumDescriptorRanges = 1;
			D3D12_DESCRIPTOR_RANGE1 range{};
			range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			range.NumDescriptors = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
			range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			range.BaseShaderRegister = 0;
			range.RegisterSpace = 0;
			table.pDescriptorRanges = &range;
			param.DescriptorTable = table;
			param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		}

		D3D12_STATIC_SAMPLER_DESC sampler_desc{};
		sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler_desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		D3D12_ROOT_SIGNATURE_DESC1 desc{};
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;
		desc.NumParameters = _countof(params);
		desc.pParameters = &params[0];
		desc.NumStaticSamplers = 1;
		desc.pStaticSamplers = &sampler_desc;

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rs_desc{};
		rs_desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		rs_desc.Desc_1_1 = desc;

		HRESULT hr{ S_OK };
		ID3DBlob* root_sig_blob{ nullptr };
		ID3DBlob* error_blob{ nullptr };
		if (FAILED(hr = D3D12SerializeVersionedRootSignature(&rs_desc, &root_sig_blob, &error_blob))) {
			DEBUG_OP(const char* error_msg{ error_blob ? (const char*)error_blob->GetBufferPointer() : "" });
			DEBUG_OP(OutputDebugStringA(error_msg));
			return;
		}
		assert(root_sig_blob);

		ID3D12RootSignature* root_sig{ nullptr };
		DXCall(hr = device()->CreateRootSignature(0, root_sig_blob->GetBufferPointer(), root_sig_blob->GetBufferSize(), IID_PPV_ARGS(&root_sig)));

		release(root_sig_blob);
		release(error_blob);

		// use root_signature
		// ...
#if 0
		ID3D12GraphicsCommandList6* cmd_list{};
		cmd_list->SetGraphicsRootSignature(root_sig);
		//only one resource heap and one sampler heap can be set
		//so max heap size is 2
		ID3D12DescriptorHeap* heaps[]{ srv_heap().heap() };
		cmd_list->SetDescriptorHeaps(1, &heaps[0]);
		//set root parameters
		float dt{ 16.6f };
		u32 dt_uint{ *((u32*)&dt) };
		u32 frame_nr{ 4287827 };
		D3D12_GPU_VIRTUAL_ADDRESS address_of_constant_buffer{};
		cmd_list->SetGraphicsRoot32BitConstant(0, dt_uint, 0);
		cmd_list->SetGraphicsRoot32BitConstant(0, frame_nr, 1);
		cmd_list->SetGraphicsRootConstantBufferView(1, address_of_constant_buffer);
		cmd_list->SetGraphicsRootDescriptorTable(2, srv_heap().gpu_start());
		//record the rest of rendering commands...
#endif

		// shutdown
		release(root_sig);
	}

	void create_a_root_signature2() {
		d3dx::d3d12_root_parameter params[3]{};
		params[0].as_constants(2, D3D12_SHADER_VISIBILITY_PIXEL, 0);
		params[1].as_cbv(D3D12_SHADER_VISIBILITY_PIXEL, 1);
		d3dx::d3d12_descriptor_range range{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV,D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,0 };
		params[2].as_descriptor_table(D3D12_SHADER_VISIBILITY_PIXEL, &range, 1);
		d3dx::d3d12_root_signature_desc root_sig_desc{ _countof(params), &params[0] };
		ID3D12RootSignature* root_sig{ root_sig_desc.create() };

		// use root_sig


	}

	ID3D12RootSignature* _root_signature;
	D3D12_SHADER_BYTECODE _vs{};

	template <D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type, typename T>
	class alignas(void*)  d3d12_pipeline_state_subobject {
	public:
		d3d12_pipeline_state_subobject() = default;
		constexpr d3d12_pipeline_state_subobject(T subobject) : _type{ type }, _subobject{ subobject }{}
		d3d12_pipeline_state_subobject& operator=(const T& subobject) {
			_subobject = subobject;
			return *this;
		}
	private:
		const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE _type{ type };
		T	_subobject{};
	};

	using d3d12_pipeline_state_subobject_root_signature = d3d12_pipeline_state_subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE, ID3D12RootSignature*>;
	using d3d12_pipeline_state_subobject_vs = d3d12_pipeline_state_subobject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS, D3D12_SHADER_BYTECODE>;

	void create_a_pipeline_state_object() {
		struct {
			struct alignas(void*) {
				const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE		type{ D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE };
				ID3D12RootSignature*							root_signature{ nullptr };
			} root_sig;
			struct alignas(void*) {
				const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE		type{ D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS };
				D3D12_SHADER_BYTECODE							vs_code{};
			} vs;
		} stream;

		stream.root_sig.root_signature = _root_signature;
		stream.vs.vs_code = _vs;

		//device()->CreateGraphicsPipelineState();
		//device()->CreateComputePipelineState();
		D3D12_PIPELINE_STATE_STREAM_DESC desc{};
		desc.pPipelineStateSubobjectStream = &stream;
		desc.SizeInBytes = sizeof(stream);

		ID3D12PipelineState* pso{ nullptr };
		device()->CreatePipelineState(&desc, IID_PPV_ARGS(&pso));

		// use pso during rendering

		// when renderer shut down
		release(pso);
	}

	void create_a_pipeline_state_object2() {
		struct {
			d3dx::d3d12_pipeline_state_subobject_root_signature root_sig{ _root_signature };
			d3dx::d3d12_pipeline_state_subobject_vs	vs{ _vs };
		} stream;

		auto pso = d3dx::create_pipeline_state(&stream, sizeof(stream));

		// use pso during rendering

		// when renderer shut down
		//release(pso);
	}