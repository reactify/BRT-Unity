using UnityEngine;

[System.Serializable]
public class DirectivityResource
{
    public string sofaFile;
    [Range(1, 15)] public int spatialResolution = 5;
}