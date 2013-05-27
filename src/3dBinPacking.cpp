//============================================================================
// Name        : 3dBinPacking.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/archive/tmpdir.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include "../include/forward_declarations.hpp"
#include "../include/cuboid.hpp"
#include "../include/shelf_algorithm.hpp"
#include "../include/guillotine2d.hpp"
#include "../include/guillotine3d.hpp"
using namespace std;

void saveXml(const std::vector<Cuboid>& cuboids, const char* filename)
{
    std::ofstream ofs(filename);
    assert(ofs.good());
    boost::archive::xml_oarchive oa(ofs);
    for (Cuboid cuboid : cuboids)
    {
        oa << boost::serialization::make_nvp("cuboid", cuboid);
    }
}

std::vector<Cuboid> loadCuboidsFromXml(const char* filename)
{

    std::ifstream ifs(filename);
    assert(ifs.good());
    boost::archive::xml_iarchive ia(ifs);

    std::vector<Cuboid> loadedCuboids;
    try
    {
    	while (true)
        {
            Cuboid cuboid;
            ia >> boost::serialization::make_nvp("cuboid",cuboid);
            loadedCuboids.push_back(cuboid);
        }
    }
    catch(boost::archive::archive_exception const& e) { return loadedCuboids;}
    return loadedCuboids;
}

vector<Cuboid> generateRandomCuboids(int number)
{
	const int maxSize = 100;
	vector<Cuboid> cuboids;
	for (int i = 0; i < number; ++i)
	{
		int width = rand() % maxSize + 1;
		int height = rand() % maxSize + 1;
		int depth = rand() % maxSize + 1;
    	cout << "Width: " << width << endl;
    	cout << "Height: " << width << endl;
    	cout << "Depth: " << width<< endl;
    	cout << "-------------" << endl;
		Cuboid c(width, width, width);
		cuboids.push_back(c);
	}
	return cuboids;
}

void usage()
{
	cout << "Usage: 3dBinPacking [-t] [-f file | -r n] [-shelf | -guillotine | -global_guillotine] width depth" << endl;
	cout << "Options:" << endl;
	cout << "-t \t: Time measurement enabled." << endl;
	cout << "-r \t: Generate n random cuboids." << endl;
	cout << "-shelf \t: Shelf algorithm + guillotine algorithm (with initial cuboids sorting)" << endl;
	cout << "-guillotine \t: Guillotine algorithm (with initial cuboids sorting)"<< endl;
	cout << "-global_guillotine \t: Global guillotine algorithm (without initial sorting)" << endl;
	cout << "-f \t: Load information about cuboids from xml file." << endl;
}

vector<Cuboid> transform(vector<Cuboid> cuboids)
{
	vector<Cuboid> tranformed;
	for (Cuboid c : cuboids)
	{
		Cuboid newCuboid;
	    newCuboid.x = c.x + (0.5 * c.width);
	    newCuboid.z = c.z + (0.5 * c.depth);
	    newCuboid.y = c.y + 0.5 * c.height;
	    newCuboid.width = c.width;
	    newCuboid.depth = c.depth;
	    newCuboid.height = c.height;
	    tranformed.push_back(newCuboid);
	}
	return tranformed;

}


void shelfAlgorithm(int binWidth, int binDepth, vector<Cuboid> cuboids, string filename)
{
	ShelfAlgorithm shelfAlg(binWidth, binDepth);
	sort(cuboids.begin(), cuboids.end(), Cuboid::compare);
	shelfAlg.insert(cuboids, ShelfAlgorithm::ShelfFirstFit);
	vector<Cuboid> placedCuboids = shelfAlg.getUsedCuboids();
	vector<Cuboid> newCuboids = transform(placedCuboids);
	saveXml(newCuboids, filename.c_str());
}

void guillotineAlgorithm(int binWidth, int binDepth, vector<Cuboid> cuboids, string filename)
{
	Guillotine3d guillotineAlg(binWidth, binDepth);
	sort(cuboids.begin(), cuboids.end(), Cuboid::compare);
	guillotineAlg.insertVector(cuboids, Guillotine3d::CuboidMinHeight, Guillotine3d::SplitLongerAxis);
	vector<Cuboid> placedCuboids = guillotineAlg.getUsedCuboids();
	vector<Cuboid> newCuboids = transform(placedCuboids);
	saveXml(newCuboids, filename.c_str());
}

void guillotineGlobalAlgorithm(int binWidth, int binDepth, vector<Cuboid> cuboids, string filename)
{
	Guillotine3d guillotineAlg(binWidth, binDepth);
	guillotineAlg.insertBestGlobalVector(cuboids, Guillotine3d::SplitLongerAxis);
	vector<Cuboid> placedCuboids = guillotineAlg.getUsedCuboids();
	vector<Cuboid> newCuboids = transform(placedCuboids);
	saveXml(newCuboids, filename.c_str());
}


int main(int argc, char* argv[])
{
	string outputFilename = "/home/krris/workspace/3dBinPacking/visualization/cuboids.xml";
	string inputFilename = "/home/krris/workspace/3dBinPacking/visualization/cuboids_input.xml";
	// Set seed
	srand (time(NULL));

	if (argc < 6 || argc > 7)
		usage();
	// Without time measurement
	else if (argc == 6)
	{
		string arg1 = argv[1];
		string alg = argv[3];
		// Generete random cuboids
		if (arg1 == "-r" &&
		   (alg == "-shelf" || alg == "-guillotine" || alg == "-global_guillotine" ))
		{
			int number = atoi(argv[2]);
			int width = atoi(argv[4]);
			int depth = atoi(argv[5]);
			cout << "-r " << number << " " << alg << " width: " << width << " depth: " << depth << endl;
			if (alg == "-shelf")
			{
				vector<Cuboid> cuboids = generateRandomCuboids(number);
				shelfAlgorithm(width, depth, cuboids, outputFilename);
			}
			else if (alg == "-guillotine")
			{
				vector<Cuboid> cuboids = generateRandomCuboids(number);
				guillotineAlgorithm(width, depth, cuboids, outputFilename);
			}
			else if (alg == "-global_guillotine")
			{
				vector<Cuboid> cuboids = generateRandomCuboids(number);
				guillotineGlobalAlgorithm(width, depth, cuboids, outputFilename);
			}

		}
		// Load cuboids from xml file.
		else if (arg1 == "-f" &&
			(alg == "-shelf" || alg == "-guillotine" || alg == "-global_guillotine" ))
		{
			string filename = argv[2];
			int width = atoi(argv[4]);
			int depth = atoi(argv[5]);
			cout << "-f " << filename << " " << alg  << " width: " << width << " depth: " << depth << endl;
			if (alg == "-shelf")
			{
				vector<Cuboid> cuboids = loadCuboidsFromXml(filename.c_str());
				shelfAlgorithm(width, depth, cuboids, outputFilename);
			}
			else if (alg == "-guillotine")
			{
				vector<Cuboid> cuboids = loadCuboidsFromXml(filename.c_str());
				guillotineAlgorithm(width, depth, cuboids, outputFilename);
			}
			else if (alg == "-global_guillotine")
			{
				vector<Cuboid> cuboids = loadCuboidsFromXml(filename.c_str());
				guillotineGlobalAlgorithm(width, depth, cuboids, outputFilename);
			}

		}
		else
			usage();
	}
	// With time measurement
	else if (argc == 7)
	{
		string arg1(argv[1]);
		string arg2 = argv[2];
		string alg = argv[4];
		// Generate random cuboids
		if (arg1 == "-t" && arg2 == "-r" &&
		   (alg == "-shelf" || alg == "-guillotine" || alg == "-global_guillotine" ))
		{
			int number = atoi(argv[3]);
			int width = atoi(argv[5]);
			int depth = atoi(argv[6]);
			cout << "-t -r " << number << " " << alg << " width: " << width << " depth: " << depth << endl;
		}
		// Load cuboids from xml file
		else if (arg1 == "-t" && arg2 == "-f" &&
		   (alg == "-shelf" || alg == "-guillotine" || alg == "-global_guillotine" ))
		{
			string filename = argv[3];
			int width = atoi(argv[5]);
			int depth = atoi(argv[6]);
			cout << "-t -f " << filename << " " << alg << "width: " << width << "depth: " << depth << endl;
		}
		else
			usage();
	}


	cout << "Works!" << endl;
	return 0;
}
