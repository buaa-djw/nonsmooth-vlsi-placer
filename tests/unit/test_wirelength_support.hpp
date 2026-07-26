#pragma once
#include "placer/objective/Wirelength.hpp"
#include <string>
inline placer::Level wireLevel(const std::vector<std::pair<double,double>>&p,const std::vector<bool>&f={}){placer::Level l;for(std::size_t i=0;i<p.size();++i)l.objects.push_back({std::to_string(i),0,0,p[i].first,p[i].second,!f.empty()&&f[i],false});return l;}
inline void addWireNet(placer::Level&l,const std::string&n,const std::vector<placer::LPin>&p){l.nets.push_back({n,p});}
