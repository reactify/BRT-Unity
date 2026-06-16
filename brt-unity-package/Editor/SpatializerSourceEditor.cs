namespace BRT.Editor
{
    using UnityEditor;
    using UnityEngine;
    
    [CustomEditor(typeof(SpatializerSource))]
    public class SpatializerSourceEditor : UnityEditor.Editor
    {
        private string[] GetSofaNames<T>(System.Collections.Generic.List<T> list) where T : class
        {
            if (list == null || list.Count == 0)
                return new[] { "(None)" };

            return list.ConvertAll(entry =>
            {
                var sofaProp = entry.GetType().GetField("sofaFile");
                return sofaProp != null ? (string)sofaProp.GetValue(entry) : "(Unknown)";
            }).ToArray();
        }
    }
}
