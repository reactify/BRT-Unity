using UnityEngine;

namespace BRT
{
    [System.Serializable]
    public class ListenerModel
    {
        public string listenerID;
        public string modelID;

        public int HRTFResourceIndex = -1;
        public int NFCResourceIndex = -1;

        public ListenerModelParameters parameters;
        
        public ListenerModel()
        {
            parameters = ListenerModelParameters.Default();
        }
    }

}