using UnityEngine;
using System.Linq;
using BRT;

public static class BRTResourceCatalog
{
    public static string[] HRTF;
    public static string[] NFC;
    public static string[] BRIR;
    public static string[] Directivity;

    private static bool _built;
    
    public const string HRTFResourceFolder = "Data/HRTF/";
    public const string BRIRResourceFolder = "Data/BRIR/";
    public const string DirectivityResourceFolder = "Data/Directivity/";
    public const string NFCFilterResourceFolder = "Data/SOSFilters/";

    public static void Rebuild()
    {
        HRTF = Load(HRTFResourceFolder);
        NFC = Load(NFCFilterResourceFolder);
        BRIR = Load(BRIRResourceFolder);
        Directivity = Load(DirectivityResourceFolder);

        _built = true;
    }

    public static string GetHRTF(int index) => Get(HRTF, index);
    public static string GetNFC(int index) => Get(NFC, index);
    public static string GetBRIR(int index) => Get(BRIR, index);
    public static string GetDirectivity(int index) => Get(Directivity, index);

    private static string Get(string[] arr, int index)
    {
        if (arr == null || index < 0 || index >= arr.Length)
            return null;

        return arr[index];
    }

    public static void Ensure()
    {
        if (!_built)
            Rebuild();
    }

    private static string[] Load(string folder)
    {
        return Resources.LoadAll<TextAsset>(folder)
            .Select(x => x.name)
            .OrderBy(x => x)
            .ToArray();
    }
}