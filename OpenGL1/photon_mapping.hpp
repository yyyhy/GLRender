#pragma once

#ifndef PHOTON_MAPPING_H
#define PHOTON_MAPPING_H
#include"glm.hpp"
#include<vector>
#include"photon.hpp"
#include"light.hpp"
#include"BVH.hpp"
#include<random>
#include<fstream>
#include<ostream>
#include"BVH.hpp"
#include<queue>
#include<functional>
#include"stb.hpp"
#include<thread>

class PhotonMapping;
PhotonMapping* pm;
#define PHOTON_MOUNT 5096*20
template<class T> using photon_queue = std::priority_queue<T, std::vector<T>,
	std::function<bool(const T&, const T&)>>;

std::thread* startGenLightMap(std::vector<Vertex>& vertices, const std::string& outfile
	, int begin, int len, PhotonMapping* pm);

inline std::vector<Vector3f> sampleVectors(unsigned size = 1024) {
	std::vector<Vector3f> vs;
	std::default_random_engine engine;
	std::uniform_real_distribution<float> real(0.0, 1.0);
	for (unsigned i = 0; i < size; ++i) {
		Vector3f sample = { real(engine) * 2.0 - 1, real(engine) * 2.0 - 1, real(engine) * 2.0 - 1 };
		sample = glm::normalize(sample);
		vs.push_back(sample);
	}
	return vs;
}

class PhotonMapping {
private:
	std::vector<Photon> photons;
	std::default_random_engine engine;
	std::uniform_int_distribution<int> dis;
	PhotonMapping() { dis = std::uniform_int_distribution<int>(10, 100); }
public:
	static PhotonMapping* GetPMInstance() {
		if (!pm)
			pm = new PhotonMapping();
		return pm;
	}

	void mappingPhotons(AccelStructrue* accStructure, Light* l, unsigned mount) {
		auto box = accStructure->GetBounds();
		std::cout << "Photon area:\n " << "x: " << box.pMin.x << " -  " << box.pMax.x << "\n"
			<< "y: " << box.pMin.y << " -  " << box.pMax.y << "\n"
			<< "z: " << box.pMin.z << " -  " << box.pMax.z << "\n";
		auto samples = sampleVectors();
		std::default_random_engine engine(time(NULL));
		std::uniform_real_distribution<float> dis(0.0, 1.0);

		unsigned index = 0;
		std::cout << "\n";
		int rec = 0;
		for (unsigned i = 0; i < mount; i++) {
			std::cout << "\r" << "mapping photons... :" << i << "/" << mount << " receive: " << rec;
			auto photon = l->samplePhoton();
			Ray r(photon.positon, photon.dir);
			auto inter = accStructure->Intersect(r);
			int depth = 0;
			while (inter.happened) {
				photon.positon = inter.coords;
				auto f = dis(engine);
				if ((f < 0.3f && depth>0) || depth > 4) {
					rec++;
					photon.times = depth;
					photons.push_back(std::move(photon));
					break;
				}
				else {
					if (glm::dot(inter.normal, photon.dir) > 0) {
						break;
					}
					double NdotL = std::max(0.f, glm::dot<float>(glm::normalize(inter.normal), -photon.dir));
					photon.color *= inter.obj->evalDiffuseColor(inter.tcoords) /
						glm::pi<double>() * NdotL;
					Vector3f out = samples[(index++) % 1024];
					if (dotProduct(out, inter.normal) < 0) {
						out = -out;
					}
					photon.dir = glm::normalize(out);
					r = Ray(photon.positon + photon.dir * 0.05f, photon.dir);
					inter = accStructure->Intersect(r);
				}
				++depth;
			}
		}
		std::cout << "\n recieve phtotons: " << rec << "\n";
	}

	void print() const {
		for (auto& i : photons) {
			std::cout << i.positon.x << " " << i.positon.y << " " << i.positon.z << "\n";
		}
	}

	void saveFile(const std::string& path) const {
		std::ofstream s;
		s.open(path);
		for (auto& p : photons) {
			s << p.positon.x << "," << p.positon.y << "," << p.positon.z << ","
				<< p.color.x << "," << p.color.y << "," << p.color.z << ","
				<< p.dir.x << "," << p.dir.y << "," << p.dir.z << "," << p.times << "\n";
		}
		s.close();
	}

	void readFile(const std::string& path) {
		std::ifstream s;
		std::string data;
		std::string dataItem;
		Photon p;
		s.open(path);
		while (std::getline(s, data)) {
			std::istringstream sin(data);
			for (int i = 0; i < 10; i++) {
				std::getline(sin, dataItem, ',');
				float v = atof(dataItem.c_str());
				switch (i)
				{
				case 0:
					p.positon.x = v;
					break;
				case 1:
					p.positon.y = v;
					break;
				case 2:
					p.positon.z = v;
					break;
				case 3:
					p.color.x = v;
					break;
				case 4:
					p.color.y = v;
					break;
				case 5:
					p.color.z = v;
					break;
				case 6:
					p.dir.x = v;
					break;
				case 7:
					p.dir.y = v;
					break;
				case 8:
					p.dir.z = v;
					break;
				case 9:
					p.times = v;
					break;
				default:
					break;
				}
			}
			photons.push_back(p);
		}

		s.close();
	}

	void genLightMapS(std::vector<Vertex>& vertices, const std::string& outfile, int& begin, int& len) {
		/*int size = vertices.size();
		std::cout <<"\nv size: " <<vertices.size()<<"\n";
		std::fstream fs(outfile);
		if (fs.is_open()) {
			std::cout << outfile << " finish" << "\n";
			std::string line;
			std::string data;
			int index = 0;
			while (std::getline(fs,line)) {
				std::istringstream sin(line);
				for (int i = 0; i < 6; i++) {
					std::getline(sin, data,' ');
					float v = atof(data.c_str());
					if (i == 0)
						vertices[index].IndLight.x = v;
					else if (i == 1)
						vertices[index].IndLight.y = v;
					else if (i == 2)
						vertices[index].IndLight.z = v;
					else if (i == 3)
						vertices[index].IndLightDir.x = v;
					else if (i == 4)
						vertices[index].IndLightDir.y = v;
					else if (i == 5)
						vertices[index].IndLightDir.z = v;
				}
				index++;
			}

			return;
		}
		else
			fs.open(outfile, std::ios::out);
		if (!fs.is_open()) {
			std::cout << "shit";
			return;
		}
		*/
		int tmpBegin = begin;
		int tmpLen = len;
		Bounds3 box;
		for (int i = tmpBegin; i < vertices.size() && i < tmpBegin + tmpLen; ++i) {
			box = Union(box, vertices[i].Position);
		}

		box.pMax *= 2;
		box.pMin *= 2;
		std::vector<Photon> photonsTmp;

		for (auto& p : photons) {
			Photon ph(p);
			photonsTmp.push_back(ph);
		}
		std::cout << tmpBegin << " " << tmpLen << " " << photonsTmp.size() << "\n";

		for (int i = tmpBegin; i < vertices.size() && i < tmpBegin + tmpLen; i++) {
			//std::cout << "\r" << "render lightmap... :" << i << "/" << vertices.size();
			Vertex v = vertices.at(i);
			photon_queue<Photon> queue([v](const Photon& photonA, const Photon& photonB) {
				float distA = glm::distance(v.Position, photonA.positon);
				float distB = glm::distance(v.Position, photonB.positon);
				float ndl = glm::dot(v.Normal, -photonA.dir);
				return distA < distB;
				});
			for (auto& p : photonsTmp)
				queue.push(p);
			float dis = 0;
			Vector3f color(0, 0, 0);
			Vector3f dir(0, 0, 0);
			float maxDis = 1;
			float cnt = 0.01;
			for (int i = photonsTmp.size() - 1; i >= photonsTmp.size() - 100 && i >= 0; --i) {
				auto& p = queue.top();
				dis = std::sqrt(dotProduct(p.positon - v.Position, p.positon - v.Position));
				maxDis = std::max(maxDis, dis);
				dir += p.dir;
				color += p.color;
				queue.pop();
				cnt++;
			}
			vertices.at(i).IndLight = color / maxDis / glm::pi<float>();
			vertices.at(i).IndLightDir = dir / cnt;

		}
		std::cout << tmpBegin << " end\n";
	}

	void genLightMap(std::vector<Vertex>& vertices, const std::string& outfile) {
		std::fstream fs(outfile);
		if (fs.is_open()) {
			std::cout << outfile << " finish" << "\n";
			std::string line;
			std::string data;
			int index = 0;
			while (std::getline(fs, line)) {
				std::istringstream sin(line);
				for (int i = 0; i < 6; i++) {
					std::getline(sin, data, ' ');
					float v = atof(data.c_str());
					if (i == 0)
						vertices[index].IndLight.x = v;
					else if (i == 1)
						vertices[index].IndLight.y = v;
					else if (i == 2)
						vertices[index].IndLight.z = v;
					else if (i == 3)
						vertices[index].IndLightDir.x = v;
					else if (i == 4)
						vertices[index].IndLightDir.y = v;
					else if (i == 5)
						vertices[index].IndLightDir.z = v;
				}
				index++;
			}

			return;
		}
		else
			fs.open(outfile, std::ios::out);
		if (!fs.is_open()) {
			std::cout << "shit";
			return;
		}

		int maxThreads = 6;
		int size = vertices.size();
		int len = size / maxThreads;
		int begin = 0;
		std::vector<std::thread*> threads;
		for (int i = 0; i < maxThreads; ++i) {
			auto t = startGenLightMap(vertices, outfile, begin, i == maxThreads - 1 ? size : len, this);
			threads.push_back(t);
			for (int j = 0; j < 1000000; ++j);
			begin += len;
		}
		for (int i = 0; i < maxThreads; ++i) {
			threads[i]->join();
		}
		for (int i = 0; i < vertices.size(); i++) {
			fs << vertices.at(i).IndLight.x << " " << vertices.at(i).IndLight.y << " " << vertices.at(i).IndLight.z << " ";
			fs << vertices.at(i).IndLightDir.x << " " << vertices.at(i).IndLightDir.y << " " << vertices.at(i).IndLightDir.z << "\n";
		}
		fs.close();
		return;
	}

	void genLightMap(AccelStructrue* stuct, std::vector<Vertex>& vertices, const std::string& outfile) {
		std::fstream fs(outfile);
		if (fs.is_open()) {
			std::cout << outfile << " finish" << "\n";
			std::string line;
			std::string data;
			int index = 0;
			while (std::getline(fs, line)) {
				std::istringstream sin(line);
				for (int i = 0; i < 6; i++) {
					std::getline(sin, data, ' ');
					float v = atof(data.c_str());
					if (i == 0)
						vertices[index].IndLight.x = v;
					else if (i == 1)
						vertices[index].IndLight.y = v;
					else if (i == 2)
						vertices[index].IndLight.z = v;
					else if (i == 3)
						vertices[index].IndLightDir.x = v;
					else if (i == 4)
						vertices[index].IndLightDir.y = v;
					else if (i == 5)
						vertices[index].IndLightDir.z = v;
				}
				index++;
			}

			return;
		}
		else
			fs.open(outfile, std::ios::out);
		if (!fs.is_open()) {
			std::cout << "shit";
			return;
		}
		std::cout << "\n";
		for (int j = 0; j < vertices.size(); j++) {
			auto& v = vertices[j];

			Vector3f irradiance;
			Vector3f irdir;
			float cnt = 0;
			for (int i = 0; i < photons.size() && cnt < 16; i++) {
				std::cout << "\r" << "gen light map IR:  " << j << "/" << vertices.size() << " " << cnt;
				auto p = photons[i];
				auto pos = p.positon;
				auto dir = glm::normalize(v.Position - p.positon);
				auto L = p.positon - v.Position;
				if (glm::dot(v.Normal, L) < 0)
					continue;
				Ray r(pos + dir * 0.05f, dir);
				auto inter = stuct->Intersect(r);
				if (inter.happened) {
					auto interPos = inter.coords;

					auto offset = interPos - v.Position;
					float dis = glm::length(offset);
					if (dis > 0.1)
						continue;
					irradiance += p.color * glm::dot(v.Normal, glm::normalize(L))
						/ glm::pi<float>() / std::powf(glm::length(L), 2);
					irdir += -L;
				}
				cnt++;
			}
			v.IndLight = irradiance;
			if (cnt > 0)
				v.IndLightDir = irdir / cnt;

		}

		for (int i = 0; i < vertices.size(); i++) {
			fs << vertices.at(i).IndLight.x << " " << vertices.at(i).IndLight.y << " " << vertices.at(i).IndLight.z << " ";
			fs << vertices.at(i).IndLightDir.x << " " << vertices.at(i).IndLightDir.y << " " << vertices.at(i).IndLightDir.z << "\n";
		}
		fs.close();
		return;
	}

	const std::vector<Photon>& getPhotons() const { return photons; }
};

std::thread* startGenLightMap(std::vector<Vertex>& vertices, const std::string& outfile
	, int begin, int len, PhotonMapping* pm) {
	std::thread* t = new std::thread(&PhotonMapping::genLightMapS, pm, std::ref(vertices),
		std::ref(outfile), std::ref(begin), std::ref(len));
	return t;
}

//for (int i = 0; i < resolution; ++i) {
		//	Vector3f left = { 0,0,0 };
		//	Vector3f right = { 0,0,0 };
		//	Vector3f leftdir = { 0,0,0 };
		//	Vector3f rightdir = { 0,0,0 };
		//	for (int j = 0; j < resolution; ++j) {
		//		if (hasData[i*resolution+j] && j < resolution - 1 && !hasData[i * resolution + j+1]) {
		//			left = getRGB(i, j, resolution, data);
		//			leftdir = getRGB(i, j, resolution, dirdata);
		//			break;
		//		}
		//	}
		//	for (int j = resolution-1; j >=0; --j) {
		//		if (hasData[i * resolution + j] && j >0 && !hasData[i * resolution + j - 1]) {
		//			right = getRGB(i, j, resolution, data);
		//			rightdir = getRGB(i, j, resolution, dirdata);
		//			break;
		//		}
		//	}
		//	//std::cout << i << " " << left.x << " " << right.x<<"\n";
		//	for (int j = 0; j < resolution; ++j) {
		//		if (!hasData[i * resolution + j]) {
		//			float leftw = 1.f - float(j) / resolution;
		//			float rightw =  float(j) / resolution;
		//			auto col = leftw * left + rightw * right;
		//			auto dir = leftw * leftdir + rightw * rightdir;
		//			setRGB(i, j, col, resolution, data);
		//			setRGB(i, j, dir, resolution, dirdata);
		//		}
		//	}
		//}


		//stbi_flip_vertically_on_write(true);
		//stbi_write_png("baking/bks.png", resolution, resolution, 3, data, 0);
		//stbi_write_png("baking/bks_dir.png", resolution, resolution, 3, dirdata, 0);
		//delete [] data;
		//delete [] hasData;
		//delete [] dirdata;
		//fs.close();
		//std::cout << "\n";
#endif // !PHOTON_MAPPING_H
