using System.IO;
using UnityEngine;

public static class ResourceExtractor
{
    /// <summary>
    /// Extract a file from Resources (as a TextAsset) to disk, preserving folder structure under persistentDataPath.
    /// Skips extraction if the file already exists on disk.
    /// </summary>
    /// <param name="resourcePath">Path inside Resources without extension, e.g. "Data/HRTF/foo.sofa"</param>
    /// <param name="outputRelativePath">Relative path inside persistentDataPath where file will be saved, e.g. "Data/HRTF/foo.sofa"</param>
    /// <param name="outFullPath">Full path of extracted file on disk</param>
    /// <returns>True if file is ready on disk (newly extracted or already existed), false if resource not found.</returns>
    public static bool ExtractToPersistentDataPath(string resourcePath, string outputRelativePath, out string outFullPath)
    {
        outFullPath = Path.Combine(Application.persistentDataPath, outputRelativePath);

        // If file already exists, skip extraction
        if (File.Exists(outFullPath))
        {
            return true;
        }

        // Load from Resources
        TextAsset asset = Resources.Load<TextAsset>(resourcePath);
        if (asset == null)
        {
            Debug.LogError($"ResourceExtractor: Could not load resource '{resourcePath}'");
            return false;
        }

        // Ensure directory exists
        string directory = Path.GetDirectoryName(outFullPath);
        if (!Directory.Exists(directory))
        {
            Directory.CreateDirectory(directory);
        }

        // Write bytes to disk
        try
        {
            File.WriteAllBytes(outFullPath, asset.bytes);
        }
        catch (System.Exception ex)
        {
            Debug.LogError($"ResourceExtractor: Failed to write file '{outFullPath}': {ex}");
            return false;
        }

        Debug.Log($"ResourceExtractor: Extracted '{resourcePath}' to '{outFullPath}'");
        return true;
    }
}
