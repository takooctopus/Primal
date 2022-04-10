using PrimalEditor.Content;

namespace PrimalEditor.Editors
{
    internal interface IAssetEditor
    {
        Asset Asset { get; }

        void SetAsset(Asset asset);
    }
}