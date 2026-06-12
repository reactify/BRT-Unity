using UnityEngine;

namespace BRT
{

    [System.Serializable]
    public class ListenerEnvironmentModel
    {
        public string listenerID;
        public string modelID;

        public int BRIRResourceIndex = -1;
        public ListenerModelParameters parameters;

        public ListenerEnvironmentModel()
        {
            parameters = ListenerModelParameters.Default();
        }
    }
}