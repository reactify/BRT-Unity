﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;



public class SphereSpawner : MonoBehaviour
{
    public GameObject spherePivot;
    public GameObject sphereEmitterPrefab;
    public bool spawnOnAwake;
    public bool spawnOnStart;
    

    private System.Diagnostics.Stopwatch stopwatch = new System.Diagnostics.Stopwatch();

    private List<GameObject> spawnedSphereEmitters = new List<GameObject>();
    public List<GameObject> SpawnedSphereEmitters
    {
        get => spawnedSphereEmitters;
    }

    public int Count => spawnedSphereEmitters.Count;

    public void Awake()
    {
        if (spawnOnAwake)
        {
            Spawn();
        }
    }

    public void Start()
    {
        if (spawnOnStart)
        {
            Spawn();
        }

        stopwatch.Start();

    }

    public void Spawn()
    {
        GameObject sphereEmitter = Instantiate(sphereEmitterPrefab, new Vector3(4, 0, 0), Quaternion.identity);
        sphereEmitter.transform.parent = spherePivot.transform;
        var audioSource = sphereEmitter.GetComponent<AudioSource>();
        audioSource.Play();
        spawnedSphereEmitters.Add(sphereEmitter);
        Debug.Log($"{spawnedSphereEmitters.Count} sources active.");
    }

    public void RemoveSource()
    {
        if (spawnedSphereEmitters.Count > 0)
        {
            Debug.Log("Removing oldest Source");
            GameObject sphere = spawnedSphereEmitters[0];
            spawnedSphereEmitters.RemoveAt(0);
            Destroy(sphere);
        }
        else
        {
            Debug.Log("Cannot remove source as none exist.");
        }
    }
}
