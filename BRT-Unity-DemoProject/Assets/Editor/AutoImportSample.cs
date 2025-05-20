using UnityEditor;
using UnityEngine;
using System.IO;

[InitializeOnLoad]
public static class AutoImportSample
{
    // This static constructor runs when the project is loaded or when scripts are recompiled
    static AutoImportSample()
    {
        CopySampleProject();
    }

    // This menu item allows you to trigger the folder copy manually
    [MenuItem("Tools/Copy Sample From Package")]
    private static void CopySampleFromPackage()
    {
        CopySampleProject(true);
    }

    private static void CopySampleProject(bool force = false)
    {
        try
        {
            // Get the project root directory
            string repoRoot = Path.GetFullPath(Path.Combine(Application.dataPath, "..", ".."));
            
            // Define paths for the package sample and the project sample folder
            string packageSamplePath = Path.Combine(repoRoot, "brt-unity-package", "Samples~", "BRT_Example");
            string projectSamplePath = Path.Combine(Application.dataPath, "Samples", "BRT_Example");

            // Log paths for debugging
            Debug.Log($"[AutoImportSample] packageSamplePath: {packageSamplePath}");
            Debug.Log($"[AutoImportSample] projectSamplePath: {projectSamplePath}");

            // Only copy if the sample doesn't already exist in the project
            if ((!Directory.Exists(projectSamplePath) && Directory.Exists(packageSamplePath)) || force)
            {
                Debug.Log("[AutoImportSample] Copying sample from package to project...");

                // Ensure the destination directory exists
                Directory.CreateDirectory(Path.GetDirectoryName(projectSamplePath));

                // Recursively copy files from package to the project
                CopyDirectory(packageSamplePath, projectSamplePath);

                // Refresh the AssetDatabase to reflect changes
                AssetDatabase.Refresh();

                Debug.Log("[AutoImportSample] Sample copied successfully.");
            }
            else
            {
                Debug.Log("[AutoImportSample] Sample already exists or package sample not found.");
            }
        }
        catch (System.Exception ex)
        {
            Debug.LogError("[AutoImportSample] Error: " + ex.Message);
        }
    }

    // Recursively copy a directory and its contents
    private static void CopyDirectory(string sourceDir, string destinationDir)
    {
        // Create all directories in the destination
        foreach (string dirPath in Directory.GetDirectories(sourceDir, "*", SearchOption.AllDirectories))
        {
            Directory.CreateDirectory(dirPath.Replace(sourceDir, destinationDir));
        }

        // Copy all files to the new destination
        foreach (string filePath in Directory.GetFiles(sourceDir, "*.*", SearchOption.AllDirectories))
        {
            string destFile = filePath.Replace(sourceDir, destinationDir);
            Directory.CreateDirectory(Path.GetDirectoryName(destFile));  // Ensure that the target directory exists
            File.Copy(filePath, destFile, overwrite: true);
        }
    }
}
