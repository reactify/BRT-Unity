using UnityEngine;

[System.Serializable]
public class ListenerModel
{
    public string ListenerID;
    public string ModelID;

    public int HRTFResourceIndex = -1;
    public int NFCResourceIndex = -1;
    public bool Enabled;
    public bool Spatialize;
    public bool Interpolation;
    public bool ITD;
    public bool ParallaxCorrection;
    public bool NearFieldEffect;

    public ListenerModel()
    {
        ListenerID = "Listener_0";
        ModelID = "Direct_Path";
        Enabled = true;
        Spatialize = true;
        Interpolation = true;
        ITD = true;
        ParallaxCorrection = false;
        NearFieldEffect = false;
        HRTFResourceIndex = 0;
        NFCResourceIndex = 0;
    }
}
