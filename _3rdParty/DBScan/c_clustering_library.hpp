// Interface for the The C clustering library

void clusterlibrary::cluster(std::vector< std::vector<float> > & data, int k, int iterations, std::vector<int> & clusterid)
{
	int nrows = data.size();
	int ncolumns = data.front().size();

	float** data_ptr = new float*[nrows];
	for(size_t i = 0; i < data.size(); i++) data_ptr[i] = &data[i][0];

	std::vector< std::vector<int> > mask(nrows, std::vector<int>(ncolumns, 1));
	int ** mask_ptr = new int*[nrows];
	for(size_t i = 0; i < mask.size(); i++) mask_ptr[i] = &mask[i][0];

	std::vector<float> data_weights(data.size(),1.0);
	float * weights = &data_weights[0];

	char distType = 'e';

	std::vector< std::vector<float> > cdata(k, std::vector<float>(ncolumns, 0));
	float ** cdata_ptr = new float*[ncolumns];
	for(size_t i = 0; i < cdata.size(); i++) cdata_ptr[i] = &cdata[i][0];

	std::vector< std::vector<int> > cmask(k, std::vector<int>(ncolumns, 1));
	int ** cmask_ptr = new int*[ncolumns];
	for(size_t i = 0; i < cmask.size(); i++) cmask_ptr[i] = &cmask[i][0];

	float err = 0;
	std::vector< int > tclusterid(nrows, 0);
	std::vector< int > counts(k, 0);
	std::vector< int > mapping(k, 0);

	clusterid.clear();
	clusterid.resize(nrows, 0);

	clusterlib::kmeans(k, nrows, ncolumns, data_ptr, mask_ptr, weights, 0, iterations, 
		distType, cdata_ptr, cmask_ptr, &clusterid[0], &err, &tclusterid[0], &counts[0], &mapping[0] );
}
