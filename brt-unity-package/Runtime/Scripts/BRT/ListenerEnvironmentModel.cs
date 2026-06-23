using UnityEngine;

namespace BRT
{
    [System.Serializable]
    public class ListenerEnvironmentModel
    {
        public string modelID;
        public string listenerID;

        public int BRIRResourceIndex = -1;
        public ListenerModelParameters parameters;

        public ListenerEnvironmentModel()
        {
            parameters = ListenerModelParameters.Default();
        }
    }
}