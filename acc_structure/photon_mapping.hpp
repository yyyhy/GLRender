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

#define PHOTON_MOUNT 5096*4
template<class T> using photon_queue = std::priority_queue<T, std::vector<T>, 
											std::function<bool(const T&, const T&)>>;

inline void setRGB(int x, int y, Vector3f alpha, unsigned resolution,unsigned char* data) {
	data[3 * (resolution * x + y) + 0] = uint8_t(alpha.x);
	data[3 * (resolution * x + y) + 1] = uint8_t(alpha.y);
	data[3 * (resolution * x + y) + 2] = uint8_t(alpha.z);
}

inline Vector3f getRGB(int x, int y, unsigned resolution, unsigned char* data) {
	return Vector3f(data[3 * (resolution * x + y) + 0],
		data[3 * (resolution * x + y) + 1],
		data[3 * (resolution * x + y) + 2]);
}

inline void setRGB(int x, int y, Vector3f alpha, unsigned resolution, char* data) {
	data[3 * (resolution * x + y) + 0] = uint8_t(alpha.x);
	data[3 * (resolution * x + y) + 1] = uint8_t(alpha.y);
	data[3 * (resolution * x + y) + 2] = uint8_t(alpha.z);
}

inline Vector3f getRGB(int x, int y, unsigned resolution, char* data) {
	return Vector3f(data[3 * (resolution * x + y) + 0],
		data[3 * (resolution * x + y) + 1],
		data[3 * (resolution * x + y) + 2]);
}


class PhotonMapping {
private:
	std::vector<Photon> photons;
	std::default_random_engine engine;
	std::uniform_int_distribution<int> dis;
	BVH* bvh=NULL;
public:
	PhotonMapping() { dis = std::uniform_int_distribution<int>(10, 100); }

	void mappingPhotons(BVH* bvh, Light* l,unsigned mount=PHOTON_MOUNT) {
		std::default_random_engine engine(time(NULL));
		std::uniform_real_distribution<float> dis(0.0, 1.0);
		std::cout << "\n";
		int rec = 0;
		for (unsigned i = 0; i < mount; i++) {
			std::cout << "\r" <<"mapping photons... :"<< i << "/" << PHOTON_MOUNT ;
			auto photon = l->samplePhoton();
			Ray r(photon.positon, photon.dir);
			auto inter = bvh->Intersect(r);
			while (inter.happened) {
				photon.positon = inter.coords;
				auto f = dis(engine);
				if (f < 0.1f) {
					rec++;
					photons.push_back(photon);
					break;
				}
				else {
					photon.dir = glm::normalize(glm::reflect(photon.dir, glm::normalize(inter.normal)));
					photon.color *= inter.obj->evalDiffuseColor(inter.tcoords);
					r = Ray(photon.positon, photon.dir);
					inter = bvh->Intersect(r);
				}	
			}
		}
		std::cout <<"\n recieve phtotons: "<<rec<<"\n";
	}

	void print() const{
		for (auto& i : photons) {
			std::cout << i.positon.x<<" " << i.positon.y << " " << i.positon.z << "\n";
		}
	}

	void saveFile() const {
		std::ofstream s;
		s.open("baking/photon.txt");
		for (auto& p : photons) {
			s << p.positon.x << "," << p.positon.y << "," << p.positon.z << ","
				<< p.color.x << "," << p.color.y << "," << p.color.z<<","
				<< p.dir.x << "," << p.dir.y << "," << p.dir.z<<"\n";
		}
		s.close();
	}

	void readFile(){
		std::ifstream s;
		std::string data;
		std::string dataItem;
		Photon p;
		s.open("baking/photon.txt");
		while (std::getline(s,data)) {
			std::istringstream sin(data);
			for (int i = 0; i < 9; i++) {
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
				default:
					break;
				}
			}
			photons.push_back(p);
		}

		s.close();
	}

	void buildBVH() {

	}

	void genLightMap(std::vector<Vertex>& vertices) const{
		int size = vertices.size();
		uint32_t resolution = std::sqrt(size);
		unsigned char* data = new unsigned char[resolution * resolution * 3]{0};
		char* dirdata = new char[resolution * resolution * 3];
		bool* hasData = new bool[resolution * resolution]{ false };
		std::cout <<"v size: " <<vertices.size()<<"\n";
		for (int i = 0; i < vertices.size();i++) {
			Vertex v = vertices.at(i);
			std::map<float, Photon> ps;
			photon_queue<Photon> q([&v](const Photon& p1, const Photon& p2)->bool {
				auto d1 = dotProduct(v.Position - p1.positon, v.Position - p1.positon);
				auto d2 = dotProduct(v.Position - p2.positon, v.Position - p2.positon);
				return d1 > d2;
				});
			for (auto& p : photons)
				q.push(p);

			float dis = 0;
			Vector3f color(0, 0, 0);
			Vector3f dir(0,0,0);
			float factors=0;
			for (int i = 0; i < 5; i++) {
				auto& p = q.top();
				dis = std::sqrt(dotProduct(p.positon - v.Position, p.positon - v.Position));
				auto col = p.color;
				color += col/dis;
				factors += 1.f/dis;
				dir += p.dir / dis;
				q.pop();
			}
			glm::vec2 uv = { v.TexCoords.x - std::floor(v.TexCoords.x) ,v.TexCoords.y - 1 - std::floor(v.TexCoords.y - 1) };
			//std::cout <<v.Position.x<<" " << v.Position.y << " " << v.Position.z << " " << uv.x << " " << uv.y << "\n";
			setRGB(resolution * uv.x, resolution * uv.y
				, color/factors*255.f, resolution, data);
			setRGB(resolution * uv.x, resolution * uv.y
				, dir/factors, resolution, dirdata);
			dir /= factors;
			std::cout << dir.x << " " << dir.y << " " << dir.z << "\n";
			int pos = resolution * uv.x * resolution + resolution * uv.y;
			hasData[pos] = true;
			vertices.at(i).IndLight = color / factors;
		}

		for (int i = 0; i < resolution; ++i) {
			Vector3f left = { 0,0,0 };
			Vector3f right = { 0,0,0 };
			Vector3f leftdir = { 0,0,0 };
			Vector3f rightdir = { 0,0,0 };
			for (int j = 0; j < resolution; ++j) {
				if (hasData[i*resolution+j] && j < resolution - 1 && !hasData[i * resolution + j+1]) {
					left = getRGB(i, j, resolution, data);
					leftdir = getRGB(i, j, resolution, dirdata);
					break;
				}
			}
			for (int j = resolution-1; j >=0; --j) {
				if (hasData[i * resolution + j] && j >0 && !hasData[i * resolution + j - 1]) {
					right = getRGB(i, j, resolution, data);
					rightdir = getRGB(i, j, resolution, dirdata);
					break;
				}
			}
			//std::cout << i << " " << left.x << " " << right.x<<"\n";
			for (int j = 0; j < resolution; ++j) {
				if (!hasData[i * resolution + j]) {
					float leftw = 1.f - float(j) / resolution;
					float rightw =  float(j) / resolution;
					auto col = leftw * left + rightw * right;
					auto dir = leftw * leftdir + rightw * rightdir;
					setRGB(i, j, col, resolution, data);
					setRGB(i, j, dir, resolution, dirdata);
				}
			}
		}


		stbi_flip_vertically_on_write(true);
		stbi_write_png("baking/bks.png", resolution, resolution, 3, data, 0);
		stbi_write_png("baking/bks_dir.png", resolution, resolution, 3, dirdata, 0);
		delete [] data;
		delete [] hasData;
		delete [] dirdata;
		std::cout << "\n";
	}

	const std::vector<Photon>& getPhotons() const { return photons; }
};
#endif // !PHOTON_MAPPING_H
