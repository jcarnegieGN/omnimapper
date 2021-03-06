#ifndef _FSR_TYPES
#define _FSR_TYPES

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/search/kdtree.h>
#include <pcl/filters/filter.h>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/array.hpp>
#include <Eigen/Dense>
#include <math.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <utility>

#define FSR_TYPES_DEBUG 0
#define FSR_TYPES_VERBOSE 0

namespace fsr_or
{
  extern Eigen::IOFormat VectorDisplayFmt;
  extern Eigen::IOFormat VectorPrintFmt;
  extern Eigen::IOFormat MatrixDisplayFmt;
  extern Eigen::IOFormat MatrixPrintFmt;

  template <typename PointT>
  struct OrientedPoint
  {
    Eigen::Vector4f p;
    Eigen::Vector4f n;

    OrientedPoint () {}

    OrientedPoint (const PointT& p, const pcl::Normal &n)
    : p (p.getVector4fMap ()),
      n (n.getNormalVector4fMap ())
    {}

    OrientedPoint (const OrientedPoint<PointT> &opp)
    : p (opp.p),
      n (opp.n)
    {}
  };

  template <typename PointT>
  struct PointPair
  {
    OrientedPoint<PointT> pu;
    OrientedPoint<PointT> pv;

    PointPair () {}

    PointPair (const OrientedPoint<PointT> &u, const OrientedPoint<PointT> &v)
    : pu (u),
      pv (v)
    {}

    PointPair (const PointT &pu, const pcl::Normal &nu, const PointT &pv, const pcl::Normal &nv)
    : pu (OrientedPoint<PointT> (pu, nu)),
      pv (OrientedPoint<PointT> (pv, nv))
    {}

    inline std::string display () const
    {
      std::stringstream ss;
      ss << "[u: (" << this->pu.p[0] << "," << this->pu.p[1] << "," << this->pu.p[2] << ")";
      ss << ",";
      ss << "(" << this->pu.n[0] << "," << this->pu.n[1] << "," << this->pu.n[2] << ")";
      ss << " and ";
      ss << "v: (" << this->pv.p[0] << "," << this->pv.p[1] << "," << this->pv.p[2] << ")";
      ss << ",";
      ss << "(" << this->pv.n[0] << "," << this->pv.n[1] << "," << this->pv.n[2] << ")]";
      return ss.str ();
    }

    inline std::string print () const
    {
      std::stringstream ss;
      ss << this->pu.p[0] << "," << this->pu.p[1] << "," << this->pu.p[2] << ",";
      ss << this->pu.n[0] << "," << this->pu.n[1] << "," << this->pu.n[2] << ",";
      ss << this->pv.p[0] << "," << this->pv.p[1] << "," << this->pv.p[2] << ",";
      ss << this->pv.n[0] << "," << this->pv.n[1] << "," << this->pv.n[2] << ",";
      return ss.str ();
    }

    static void read (std::string &sopp, PointPair<PointT> &opp)
    {
      std::stringstream ss (sopp);
      std::string value;

      std::getline (ss, value, ','); opp.pu.p[0] = std::stof (value);
      std::getline (ss, value, ','); opp.pu.p[1] = std::stof (value);
      std::getline (ss, value, ','); opp.pu.p[2] = std::stof (value);
      std::getline (ss, value, ','); opp.pu.n[0] = std::stof (value);
      std::getline (ss, value, ','); opp.pu.n[1] = std::stof (value);
      std::getline (ss, value, ','); opp.pu.n[2] = std::stof (value);

      std::getline (ss, value, ','); opp.pv.p[0] = std::stof (value);
      std::getline (ss, value, ','); opp.pv.p[1] = std::stof (value);
      std::getline (ss, value, ','); opp.pv.p[2] = std::stof (value);
      std::getline (ss, value, ','); opp.pv.n[0] = std::stof (value);
      std::getline (ss, value, ','); opp.pv.n[1] = std::stof (value);
      std::getline (ss, value, ','); opp.pv.n[2] = std::stof (value);
    }
  };

  template <typename PointT>
  struct PointPairSystem
  {
    PointPair<PointT> opp;
    Eigen::Matrix4f F;

    PointPairSystem () {}

    PointPairSystem (const PointPair<PointT> &opp, const Eigen::Matrix4f &F)
    : opp (opp),
      F (F)
    {}

    PointPairSystem (const PointPair<PointT> &opp)
    : opp (opp),
      F ()
    {}

    inline std::string display () const
    {
      std::stringstream ss;
      ss << this->opp.display () << "\n" << this->F.format (MatrixDisplayFmt) << "\n";
      return ss.str();
    }

    inline std::string print () const
    {
      std::stringstream ss;
      Eigen::Matrix<float, 3, 4> mat;
      mat.block(0,0,3,4) = this->F.block(0,0,3,4);
      ss << this->opp.print () << mat.format (MatrixPrintFmt) << "\n";
      return ss.str();
    }

    static bool read (std::string &spps, PointPairSystem<PointT> &pps)
    {
      if (spps.size () < 12 || spps.find (':') != std::string::npos)
      {
        return false;
      }

      PointPair<PointT>::read (spps, pps.opp);

      std::stringstream ss (spps);
      std::string value;
      for (size_t i = 0; i < 12; ++i)
      {
        std::getline (ss, value, ',');
        pps.F(i / 4, i % 4) = std::stof (value);
      }
      pps.F(3, 0) = 0;
      pps.F(3, 1) = 0;
      pps.F(3, 2) = 0;
      pps.F(3, 3) = 1;

      return true;
    }
  };

  struct FSRFeature : public std::pair<int, std::pair<int,int> >
  {
    FSRFeature () {}

    /// f1 is constant
    FSRFeature (int f2, int f3, int f4)
    : std::pair<int, std::pair<int,int> > (f2, std::pair<int,int> (f3, f4))
    {}

    inline std::string print () const
    {
      std::stringstream ss;
      ss << this->first << "," << this->second.first << "," << this->second.second;
      return ss.str ();
    }

    static bool read (std::string &sf, FSRFeature &f)
    {
      std::stringstream ss (sf);
      std::string value;

      std::getline (ss, value, ',');
      f.first = std::stoi (value);
      std::getline (ss, value, ',');
      f.second.first = std::stoi (value);
      std::getline (ss, value);
      f.second.second = std::stoi (value);

      return true;
    }
  };

  template <typename PointT>
  bool computeOPFeature (OrientedPoint<PointT> &u, OrientedPoint<PointT> &v,
                         float &f1, float &f2, float &f3, float &f4,
                         const float &dminSq, const float &dmaxSq)
  {
    Eigen::Vector4f &pu = u.p;
    Eigen::Vector4f &pv = v.p;

    float dx = pv[0] - pu[0];
    float dy = pv[1] - pu[1];
    float dz = pv[2] - pu[2];
    f1 = dx*dx + dy*dy + dz*dz;

    if (f1 == 0.0f)
    {
      #if FSR_TYPES_DEBUG
      std::cout << "[fsr_or::computeFeature()]: pu -> " << pu.format(VectorDisplayFmt) << " and pv -> "
                << pu.format(VectorDisplayFmt) << " are the same point." << std::endl;
      #endif // FSR_TYPES_DEBUG
      f1 = f2 = f3 = f4 = 0.0;
      return false;
    }
    else if (f1 < dminSq || f1 > dmaxSq)
    {
      #if FSR_TYPES_VERBOSE
      std::cout << "[fsr_or::computeFeature()]: pu -> " << pu.format(VectorDisplayFmt) << " and pv -> "
                << pu.format(VectorDisplayFmt) << " are too far apart." << std::endl;
      #endif // FSR_TYPES_VERBOSE
      f2 = f3 = f4 = 0.0;
      return false;
    }

    f1 = sqrtf(f1);

    Eigen::Vector4f &nu = u.n;
    Eigen::Vector4f &nv = v.n;
    f2 = acos (nu[0]*nv[0] + nu[1]*nv[1] + nu[2]*nv[2]);

    /// n_u dot (pv-pu)
    float alpha_u = acos (nu[0]*dx + nu[1]*dy + nu[2]*dz);
    /// n_v dot (pu-pv)
    float alpha_v = acos (-nv[0]*dx - nv[1]*dy - nv[2]*dz);
    if (alpha_u < alpha_v)
    {
      f3 = alpha_u;
      f4 = alpha_v;
    }
    else
    {
      /// switch pu and pv
      f3 = alpha_v;
      f4 = alpha_u;

      /// switch v and u because other algorithms depend on the ordering
      OrientedPoint<PointT> copy_u = u;
      OrientedPoint<PointT> copy_v = v;
      v = copy_u;
      u = copy_v;
    }

    return true;
    //return pcl::computePairFeature(pu, nu, pv, nv, f.first, f.second.first, f.second.second, f1);
  }

  inline void discretizeOPFeatureVals (const float &f2, const float &f3, const float &f4,
                                       int &d2, int &d3, int &d4,
                                       const float &step)
  {
    d2 = static_cast<int> (floor (f2 * step));
    d3 = static_cast<int> (floor (f3 * step));
    d4 = static_cast<int> (floor (f4 * step));
  }

  template <typename PointT>
  bool computeF (const PointPair<PointT> &uv, Eigen::Matrix4f &F)
  {
    Eigen::Vector4f puv = uv.pv.p - uv.pu.p;

    Eigen::Vector4f pcrossn = puv.cross3 (uv.pv.n + uv.pu.n);
    Eigen::Vector4f pcrossncrossuv = pcrossn.cross3 (puv);

    F.block(0,0,4,1) = pcrossn/pcrossn.norm ();
    F.block(0,1,4,1) = puv/puv.norm ();
    F.block(0,2,4,1) = pcrossncrossuv/pcrossncrossuv.norm ();
    F.block(0,3,4,1) = (uv.pv.p + uv.pu.p) / 2.0;

    return true;
  }

  template <typename PointT>
  double computeCloudResolution (typename pcl::PointCloud<PointT>::Ptr &cloud)
  {
    double resolution = 0.0;
	int numberOfPoints = 0;
	int nres;
	std::vector<int> indices (2);
	std::vector<float> squaredDistances (2);
	pcl::search::KdTree<PointT> tree;
	tree.setInputCloud (cloud);

	for (size_t i = 0; i < cloud->size (); ++i)
	{
		if (!pcl_isfinite ((*cloud)[i].x))
		{
		  continue;
		}

        nres = tree.nearestKSearch (i, 2, indices, squaredDistances);
		if (nres == 2)
		{
			resolution += sqrt (squaredDistances[1]);
			++numberOfPoints;
		}
	}
	if (numberOfPoints != 0)
	{
	  resolution /= numberOfPoints;
	}

	return resolution;
  }

  template <typename PointT>
  int scaleCloudToResolution (typename pcl::PointCloud<PointT>::Ptr &cloud_in,
                              typename pcl::PointCloud<PointT>::Ptr &cloud_out,
                              const double &outres)
  {
    cloud_out = typename pcl::PointCloud<PointT>::Ptr (new pcl::PointCloud<PointT> ());

    double inres = fsr_or::computeCloudResolution<PointT> (cloud_in);
    double multiplier = outres / inres;

    size_t numpts = cloud_in->points.size ();
    size_t nummoved = 0;
    size_t i = 0;
    boost::unordered_set<size_t> process_next;
    boost::unordered_set<size_t> process_waiting;
    boost::unordered_set<size_t> process_done;
    boost::unordered_set<size_t>::iterator it;
    int k = 8, l = 0, nfound = 0;
    PointT pt1, pt2, anchor, newpt;
    std::vector<int> indices (k);
    std::vector<float> sqDists (k);
    pcl::search::KdTree<PointT> tree;
    tree.setInputCloud (cloud_in);

    newpt.x = newpt.y = newpt.z = -1.0f/0.0f;
    cloud_out->width = static_cast<int> (numpts);
    cloud_out->height = 1;
    cloud_out->points.resize (numpts, newpt);

    while (!pcl::isFinite (cloud_in->points[i]))
    {
      cloud_out->is_dense = false;
      ++nummoved;
      ++i;
    }

    process_waiting.insert (i);
    cloud_out->points[i] = cloud_in->points[i];
    ++nummoved;
    while (process_waiting.size () > 0)
    {
      process_next = process_waiting;
      process_waiting.clear ();

      for (it = process_next.begin (); it != process_next.end (); ++it)
      {
        i = *it;
        pt1 = cloud_in->points[i];
        anchor = cloud_out->points[i];

        nfound = tree.nearestKSearch (pt1, k, indices, sqDists);

        for (size_t j = 0; j < nfound; ++j)
        {
          l = indices[j];

          pt2 = cloud_in->points[l];

          if (!pcl::isFinite (pt2))
          {
            cloud_out->is_dense = false;
            process_done.insert (l);
            ++nummoved;
            continue;
          }
          if (process_done.find (l) != process_done.end () ||
              process_next.find (l) != process_next.end () ||
              process_waiting.find (l) != process_waiting.end ())
          {
            continue;
          }

          cloud_out->points[l].x = anchor.x + multiplier * (pt2.x - pt1.x);
          cloud_out->points[l].y = anchor.y + multiplier * (pt2.y - pt1.y);
          cloud_out->points[l].z = anchor.z + multiplier * (pt2.z - pt1.z);

          process_waiting.insert (l);
          ++nummoved;
        }

        process_done.insert(i);
      }
    }

    std::vector<int> finites;
    pcl::removeNaNFromPointCloud (*cloud_out, *cloud_out, finites);

    return static_cast<int> (finites.size ());
  }

  struct OMKey : public std::pair<std::string, int>
  {
    typedef boost::shared_ptr<OMKey> Ptr;

    OMKey () {}

    OMKey (const std::string &m, const int &t)
    : std::pair<std::string, int> (m, t)
    {}

    inline std::string print () const
    {
      std::stringstream ss;
      ss << this->first << "," << this->second;
      return ss.str ();
    }

    inline std::string objectFileName () const
    {
      std::stringstream ss;
      /// .../database/model/model_type/model_type.pcd
      ss << "/home/jessie/Desktop/3d_object_recognition/fsr_3d_object_recognition/database/"
         << this->first << "/" << this->first << "_" << this->second << "/"
         << this->first << "_" << this->second << ".pcd";
      return ss.str ();
    }

    inline std::string objectName () const
    {
      std::stringstream ss;
      /// model_type
      ss << this->first << "_" << this->second;
      return ss.str ();
    }

    Ptr makeShared () const { return Ptr (new OMKey (*this)); }
  };

  template <typename T>
  class ObjectMap : public boost::unordered_map<OMKey, T>
  {
    public:
      typedef boost::shared_ptr<ObjectMap> Ptr;
      typedef typename ObjectMap::iterator iterator;

      inline void insert_model (const OMKey &mkey, const T &val)
      {
        (*this)[mkey] = T (val);
      }

      inline iterator find_model (const OMKey &mkey)
      {
        return this->find (mkey);
      }

      Ptr makeShared () const { return Ptr (new ObjectMap (*this)); }
  };

  template <typename PointT>
  class FeatureHashMap : public boost::unordered_map<FSRFeature, ObjectMap<std::vector<PointPairSystem<PointT> > > >
  {
    public:
      typedef boost::shared_ptr<FeatureHashMap> Ptr;

      inline void insert_model (const FSRFeature &f, const OMKey &mkey, const std::vector<PointPairSystem<PointT> > &syss)
      {
        ((*this)[f]).insert_model (mkey, syss);
      }

      inline typename ObjectMap<std::vector<PointPairSystem<PointT> > >::iterator find_model (const FSRFeature &f, const OMKey &mkey)
      {
        return ((*this)[f]).find_model (mkey);
      }

      inline typename ObjectMap<std::vector<PointPairSystem<PointT> > >::iterator begin_model (const FSRFeature &f)
      {
        return ((*this)[f]).begin ();
      }

      inline typename ObjectMap<std::vector<PointPairSystem<PointT> > >::iterator end_model (const FSRFeature &f)
      {
        return ((*this)[f]).end ();
      }

      Ptr makeShared () const { return Ptr (new FeatureHashMap (*this)); }
  };

  template<typename T>
  struct HashCompare
  {
    static size_t hash(const T &key)
    {
      return boost::hash_value(key);
    }

    static bool equal(const T &key1, const T &key2)
    {
      return ( key1 == key2 );
    }
  };

  extern bool operator== (OMKey::Ptr const &om1, OMKey::Ptr const &om2);

  struct om_hash : public std::unary_function<OMKey::Ptr, std::size_t>
  {
    std::size_t operator()(OMKey::Ptr const& om) const
    {
      return boost::hash_value<OMKey> (*om);
    }
  };

  struct OMHashCompare
  {
    static size_t hash(const OMKey::Ptr &key)
    {
      return boost::hash_value(*key);
    }

    static bool equal(const OMKey::Ptr &key1, const OMKey::Ptr &key2)
    {
      return ( (*key1) == (*key2) );
    }
  };

}

#endif
