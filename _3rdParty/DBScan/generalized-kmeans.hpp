// Incomplete..

//Generalized k-means
//Copyright (C) 2013 Balazs Szalkai
//If you use this program in your research, please cite the following article:
//B. Szalkai: An implementation of the relational k-means algorithm. ArXiv e-prints, 2013.

namespace GeneralizedKMeans
{
	typedef Eigen::MatrixXf DistanceMatrix;

	struct Clusters
	{
		DistanceMatrix Matrix;
		int Count;
		int[] ObjectToCluster;
		List<int>[] ClusterToObjects;
		CentroidDistance cd;

		Clusters(DistanceMatrix & matrix, int count)
		{
			Matrix = matrix;
			Count = count;
			ObjectToCluster = new int[Matrix.Count];
			ClusterToObjects = new List<int>[Count];
			for (int i = 0; i < Count; i++) ClusterToObjects[i] = new List<int>();
			Randomize();
		}

		static Random Rand = new Random();

		void Randomize()
		{
			for (int i = 0; i < Count; i++) ClusterToObjects[i].Clear();

			for (int i = 0; i < Matrix.Count; i++)
			{
				int cluster = Rand.Next(Count);
				ObjectToCluster[i] = cluster;
				ClusterToObjects[cluster].Add(i);
			}
		}

		void Make_ClusterToObjects()
		{
			for (int cluster = 0; cluster < Count; cluster++)
				ClusterToObjects[cluster].Clear();

			for (int i = 0; i < Matrix.Count; i++)
				ClusterToObjects[ObjectToCluster[i]].Add(i);
		}

		double[] AdditiveConstant;

		// Call this before using the other members functions
		void Initialize()
		{
			// calculate additive constants for clusters
			AdditiveConstant = new double[clusters.Count];
			for (int cluster = 0; cluster < clusters.Count; cluster++)
			{
				var objects = clusters.ClusterToObjects[cluster];
				double x = 0;
				foreach (var i in objects)
					foreach (var j in objects)
					x += clusters.Matrix.SqrDistance[i, j];
				AdditiveConstant[cluster] = -x / (2 * Utils.Sqr((double)objects.Count));
			}
		}
		// Calculates a squared centroid distance
		double CalculateSqrDist(int obj, int cluster)
		{
			var objects = clusters.ClusterToObjects[cluster];
			double x = 0;
			foreach (int j in objects) x += clusters.Matrix.SqrDistance[obj, j];
			double dist = AdditiveConstant[cluster] + x / objects.Count;
			return dist;
		}
		int GetNearestCluster(int obj)
		{
			int nearestCluster = -1;
			double minSqrDist = 0;
			for (int cluster = 0; cluster < clusters.Count; cluster++)
			{
				double sqrDist = CalculateSqrDist(obj, cluster);
				if (nearestCluster < 0 || sqrDist < minSqrDist)
				{
					minSqrDist = sqrDist;
					nearestCluster = cluster;
				}
			}
			return nearestCluster;
		}
		double GetClusteringValue()
		{
			double result = 0;
			for (int i = 0; i < clusters.Matrix.Count; i++)
				result += CalculateSqrDist(i, clusters.ObjectToCluster[i]);
			return result;
		}

		// Returns how many objects have changed cluster
		int Iterate()
		{
			int changed = 0;
			cd.Initialize();

			// find the nearest centroid for the objects
			int[] New_ObjectToCluster = new int[Matrix.Count];
			for (int i = 0; i < Matrix.Count; i++)
			{
				New_ObjectToCluster[i] = cd.GetNearestCluster(i);
				if (New_ObjectToCluster[i] != ObjectToCluster[i]) changed++;
			}

			// update the configuration
			double oldValue = GetValue();
			int[] Old_ObjectToCluster = ObjectToCluster;
			ObjectToCluster = New_ObjectToCluster;
			Make_ClusterToObjects();
			double newValue = GetValue();

			// clustering got worse (this is possible with some distance matrices) or stayed the same?
			if (oldValue <= newValue)
			{
				// undo iteration
				ObjectToCluster = Old_ObjectToCluster;
				Make_ClusterToObjects();
				return 0;
			}
			return changed;
		}

		double GetValue()
		{
			cd.Initialize();
			return cd.GetClusteringValue();
		}
	}

	void Run( DistanceMatrix & m, int nClusters )
	{
		Clusters bestClusters (m, nClusters);

		int badLuckStreak = 0;
		int blockId = 0;
		int maxBadLuckStreak = 100;

		while (true)
		{
			for (int i = 0; i < nThreads; i++)
			{
				Clusters c = new Clusters(m, nClusters);
				while (true)
				{
					if (c.Iterate() == 0) break;
				}

				Clusters c = clustersArray[i];
				if (c.GetValue() < bestClusters.GetValue())
				{
					bestClusters = c;
					badLuckStreak = 0;
				}
				else 
					badLuckStreak++;
			}

			if (badLuckStreak >= maxBadLuckStreak) break;
		}
	}
}
