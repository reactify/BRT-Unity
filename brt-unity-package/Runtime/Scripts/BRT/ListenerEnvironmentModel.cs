using UnityEngine;

[System.Serializable]
public class ListenerEnvironmentModel
{
    public string ListenerID;
    public string ModelID;

    public int BRIRResourceIndex = -1;
    public bool Enabled;

    public ListenerEnvironmentModel()
    {
        ListenerID = "Listener_0";
        ModelID = "Reverb_Path";
        Enabled = true;
        BRIRResourceIndex = 0;
    }
}