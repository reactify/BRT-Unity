using UnityEngine;

namespace BRT
{
    [System.Serializable]
    public class ListenerModel
    {
        public string modelID;
        public string listenerID;

        public int HRTFResourceIndex = -1;
        public int NFCResourceIndex = -1;

        public ListenerModelParameters parameters;
        
        public ListenerModel()
        {
            parameters = ListenerModelParameters.Default();
        }
    }

}