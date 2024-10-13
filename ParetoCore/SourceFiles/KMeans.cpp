/*
///*********************************************************************
double ContourExtraction::dist2(kmeans_point a, kmeans_point b)
{
	double x = a->x - b->x, y = a->y - b->y;
	return x*x + y*y;
}
//**********************************************************************
int ContourExtraction::nearest(kmeans_point pt, kmeans_point cent, int n_cluster, double *d2)
{
	int i, min_i;
	kmeans_point c;
	double d, min_d;

	for (c = cent, i = 0; i < n_cluster; i++, c++) {
		min_d = HUGE_VAL;
		min_i = pt->group;
		for (c = cent, i = 0; i < n_cluster; i++, c++) {
			if (min_d > (d = dist2(c, pt))) {
				min_d = d; min_i = i;
			}
		}
	}
	if (d2) *d2 = min_d;
	return min_i;
}
//**********************************************************************
double randf(double m)
{
	return m * rand() / (RAND_MAX - 1.);
}
//**********************************************************************
void ContourExtraction::kpp(kmeans_point *pts, int len, kmeans_point *cent, int n_cent)
{
	// Initialization for kmeans clustering
	// Input: The pts that need to be classified
	// Output: The group of each point (see kmeans_point structure)

	int j;
	int n_cluster;
	double sum, *d;
	kmeans_point p;

	d = (double *)malloc(sizeof(double) * len);
	cent[0] = pts[ rand() % len ];// pick a random point as center of 1st cluster
	for (n_cluster = 1; n_cluster < n_cent; n_cluster++) {
		sum = 0;
		for (j = 0; j < len; j++) {
			p = pts[j];
			nearest(p, cent, n_cluster, d + j);
			sum += d[j];
		}
		sum = randf(sum);
		for (j = 0, p = pts; j < len; j++, p++) {
			if ((sum -= d[j]) > 0) continue;
			cent[n_cluster] = pts[j];
			break;
		}
	}
	for (j = 0; j < len; j++){
		p = pts[j];
		p->group = nearest(p, cent, n_cluster, 0);
	}
	free(d);
}
//**********************************************************************
kmeans_point ContourExtraction::lloyd(kmeans_point *pts, int len, int n_cluster)
{
	// Input: The pts that need to be classified, the number of points, and desired number of clusters
	// Output: The group of each point (see kmeans_point structure)
	int i, j, min_i;
	int changed;
	kmeans_point p,c;
 
	kmeans_point *cent = (kmeans_point *) malloc(sizeof(kmeans_point) * n_cluster);
 
	kpp(pts, len, cent, n_cluster);//init
 
	do {
		// group element for centroids are used as counters 
		for (c = cent, i = 0; i < n_cluster; i++, c++) { 
			c->group = 0; 
			c->x = c->y = 0; 
		}
		for (j = 0, p = pts; j < len; j++, p++) {
			c = cent + p->group;
			c->group++;
			c->x += p->x; c->y += p->y;
		}
		for (c = cent, i = 0; i < n_cluster; i++, c++) { 
			c->x /= c->group; 
			c->y /= c->group; 
		}
 
		changed = 0;
		// find closest centroid of each point 
		for (j = 0, p = pts; j < len; j++, p++) {
			min_i = nearest(p, cent, n_cluster, 0);
			if (min_i != p->group) {
				changed++;
				p->group = min_i;
			}
		}
	} while (changed > (len >> 10)); // stop when 99.9% of points are good 
 
	for (c = cent, i = 0; i < n_cluster; i++, c++) { 
		c->group = i; 
	}
 
	return cent;
}*/