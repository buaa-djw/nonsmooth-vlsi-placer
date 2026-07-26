// Consolidated from test_bookshelf_aux.cpp
#include "placer/io/BookshelfReader.hpp"
#include "../TestSupport.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>
namespace fs=std::filesystem;
int consolidated_bookshelf_0(){const fs::path root="output/stage1_aux";fs::remove_all(root);fs::create_directories(root/"input files");for(const auto&name:{"case.nodes","case.nets","case.pl","case.scl"})std::ofstream(root/"input files"/name);std::ofstream(root/"case.aux")<<"RowBasedPlacement :\n\"input files/case.scl\" \"input files\\case.pl\"\n\"input files/case.nets\" \"input files/case.nodes\"\n";auto f=placer::parseAux((root/"case.aux").string());CHECK_EQ(f.size(),4u);CHECK(f.at(".nodes").find("input files/case.nodes")!=std::string::npos);std::ofstream(root/"other.nodes");std::ofstream(root/"conflict.aux")<<"RowBasedPlacement : \"input files/case.nodes\" other.nodes \"input files/case.nets\" \"input files/case.pl\" \"input files/case.scl\"\n";bool conflict=false;try{(void)placer::parseAux((root/"conflict.aux").string());}catch(const std::runtime_error&e){const std::string m=e.what();conflict=m.find("conflicting .nodes")!=std::string::npos&&m.find("conflict.aux")!=std::string::npos;}CHECK(conflict);return 0;}

// Consolidated from test_bookshelf_counts.cpp
#include "placer/io/BookshelfReader.hpp"
#include "../TestSupport.hpp"
#include <filesystem>
#include <iostream>
int consolidated_bookshelf_1(){
    auto db=placer::loadBookshelf("tests/data/stage1/valid/valid.aux");
    CHECK(db.declared_counts.num_nodes && *db.declared_counts.num_nodes==4); CHECK(db.declared_counts.num_terminals && *db.declared_counts.num_terminals==1);
    CHECK(db.declared_counts.num_nets && *db.declared_counts.num_nets==2); CHECK(db.declared_counts.num_pins && *db.declared_counts.num_pins==4); CHECK(db.declared_counts.num_rows && *db.declared_counts.num_rows==1);
    CHECK_EQ(db.cells.size(),4u); CHECK_EQ(db.nets.size(),2u); CHECK_EQ(db.pins.size(),4u); CHECK_EQ(db.rows.size(),2u);
    const std::filesystem::path aux="testbench/ispd2005/adaptec1/adaptec1.aux"; if(!std::filesystem::exists(aux)){std::cout<<"SKIP adaptec1 integration: benchmark unavailable\n";return 0;}
    auto a=placer::loadBookshelf(aux.string()); std::size_t fixed=0;for(const auto&c:a.cells)fixed+=c.fixed?1u:0u;
    CHECK_EQ(a.cells.size(),211447u);CHECK_EQ(fixed,543u);CHECK_EQ(a.nets.size(),221142u);CHECK_EQ(a.pins.size(),944053u);CHECK_EQ(a.rows.size(),890u);return 0;}

// Consolidated from test_bookshelf_gzip.cpp
#include "placer/io/BookshelfReader.hpp"
#include "placer/io/TextInput.hpp"
#include "../TestSupport.hpp"
#include <zlib.h>
int consolidated_bookshelf_2(){
 const fs::path root=fs::temp_directory_path()/("nonsmooth_gzip_"+std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()));fs::create_directories(root);
 for(const auto&name:{"valid.nets","valid.pl","valid.scl"})fs::copy_file(fs::path("tests/data/stage1/valid")/name,root/name);
 const auto nodes=placer::readTextFile("tests/data/stage1/valid/valid.nodes");gzFile gz=gzopen((root/"valid.nodes.gz").c_str(),"wb");CHECK(gz!=nullptr);CHECK_EQ(gzwrite(gz,nodes.data(),static_cast<unsigned>(nodes.size())),static_cast<int>(nodes.size()));CHECK_EQ(gzclose(gz),Z_OK);
 std::ofstream(root/"valid.aux")<<"RowBasedPlacement : valid.nodes.gz valid.nets valid.pl valid.scl\n";
 auto files=placer::parseAux((root/"valid.aux").string());CHECK(files.at(".nodes").find(".nodes.gz")!=std::string::npos);auto db=placer::loadBookshelf((root/"valid.aux").string());CHECK_EQ(db.cells.size(),4u);fs::remove_all(root);return 0;}


// Consolidated from test_bookshelf_invalid.cpp
#include "placer/io/BookshelfReader.hpp"
#include "../TestSupport.hpp"
#include <filesystem>
#include <fstream>
#include <string>
namespace fs=std::filesystem;
static const char* scl="UCLA scl 1.0\nNumRows : 1\nCoreRow Horizontal\nCoordinate : 0\nHeight : 1\nSitewidth : 1\nSitespacing : 1\nSiteorient : N\nSitesymmetry : Y\nSubrowOrigin : 0 NumSites : 2\nEnd\n";
static bool rejected(const fs::path&root,const std::string&nodes,const std::string&pl,const std::string&nets,const std::string&scl_text,const std::string&needle){fs::remove_all(root);fs::create_directories(root);std::ofstream(root/"bad.aux")<<"RowBasedPlacement : bad.nodes bad.nets bad.pl bad.scl\n";std::ofstream(root/"bad.nodes")<<nodes;std::ofstream(root/"bad.pl")<<pl;std::ofstream(root/"bad.nets")<<nets;std::ofstream(root/"bad.scl")<<scl_text;try{(void)placer::loadBookshelf((root/"bad.aux").string());}catch(const std::runtime_error&e){return std::string(e.what()).find(needle)!=std::string::npos;}return false;}
int consolidated_bookshelf_3(){const fs::path r="output/stage1_invalid";const std::string good_nodes="UCLA nodes 1.0\nNumNodes : 1\nNumTerminals : 0\na 1 1\n",good_pl="UCLA pl 1.0\na 0 0 : N\n",good_nets="UCLA nets 1.0\nNumNets : 1\nNumPins : 1\nNetDegree : 1 n\na I : 0 0\n";
CHECK(rejected(r,"UCLA nodes 1.0\nNumNodes : 2\nNumTerminals : 0\na 1 1\n",good_pl,good_nets,scl,"NumNodes mismatch"));
CHECK(rejected(r,"UCLA nodes 1.0\nNumNodes : 1\nNumTerminals : 1\na 1 1\n",good_pl,good_nets,scl,"NumTerminals mismatch"));
CHECK(rejected(r,good_nodes,good_pl,"UCLA nets 1.0\nNumNets : 2\nNumPins : 1\nNetDegree : 1 n\na I : 0 0\n",scl,"NumNets mismatch"));
CHECK(rejected(r,good_nodes,good_pl,"UCLA nets 1.0\nNumNets : 1\nNumPins : 2\nNetDegree : 1 n\na I : 0 0\n",scl,"NumPins mismatch"));
CHECK(rejected(r,good_nodes,good_pl,"UCLA nets 1.0\nNumNets : 1\nNumPins : 2\nNetDegree : 2 n\na I : 0 0\n",scl,"premature EOF"));
CHECK(rejected(r,good_nodes,good_pl,"UCLA nets 1.0\nNumNets : 2\nNumPins : 2\nNetDegree : 2 n\na I : 0 0\nNetDegree : 1 next\na I : 0 0\n",scl,"NetDegree mismatch"));
CHECK(rejected(r,"UCLA nodes 1.0\nNumNodes : 2\nNumTerminals : 0\na 1 1\na 1 1\n",good_pl,good_nets,scl,"duplicate cell"));
CHECK(rejected(r,good_nodes,good_pl,"UCLA nets 1.0\nNumNets : 1\nNumPins : 1\nNetDegree : 1 n\nmissing I : 0 0\n",scl,"unknown pin cell"));
CHECK(rejected(r,good_nodes,"UCLA pl 1.0\nmissing 0 0 : N\n",good_nets,scl,"unknown PL cell"));
CHECK(rejected(r,"UCLA nodes 1.0\nNumNodes : 1\nNumTerminals : 0\na 1x 1\n",good_pl,good_nets,scl,"invalid width"));
CHECK(rejected(r,"UCLA nodes 1.0\nNumNodes : 1\nNumTerminals : 0\na NaN 1\n",good_pl,good_nets,scl,"invalid width"));
CHECK(rejected(r,"UCLA nodes 1.0\nNumNodes : 1\nNumTerminals : 0\na +Inf 1\n",good_pl,good_nets,scl,"invalid width"));
CHECK(rejected(r,"UCLA nodes 1.0\nNumNodes : 1\nNumTerminals : 0\na 0 1\n",good_pl,good_nets,scl,"non-positive"));
CHECK(rejected(r,good_nodes,good_pl,good_nets,"UCLA scl 1.0\nNumRows : 1\nCoreRow Horizontal\nCoordinate : 0\nEnd\n","missing required"));
fs::remove_all(r);fs::create_directories(r);std::ofstream(r/"bad.aux")<<"RowBasedPlacement : bad.nodes bad.nets bad.pl\n";std::ofstream(r/"bad.nodes");std::ofstream(r/"bad.nets");std::ofstream(r/"bad.pl");bool aux=false;try{(void)placer::parseAux((r/"bad.aux").string());}catch(const std::runtime_error&e){aux=std::string(e.what()).find("missing .scl reference")!=std::string::npos;}CHECK(aux);return 0;}

// Consolidated from test_bookshelf_reader.cpp
#include "placer/io/BookshelfReader.hpp"
#include "../TestSupport.hpp"
int consolidated_bookshelf_4(){ auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux"); CHECK_EQ(db.cells.size(),3u); CHECK_EQ(db.nets.size(),2u); CHECK_EQ(db.pins.size(),5u); auto r=db.region(); CHECK_NEAR(r.xl,0.0,1e-12); CHECK_NEAR(r.xh,20.0,1e-12); CHECK_EQ(db.rows.size(),2u); return 0; }

// Consolidated from test_fixed_terminal.cpp
#include "placer/io/BookshelfReader.hpp"
#include "../TestSupport.hpp"
int consolidated_bookshelf_5(){auto db=placer::loadBookshelf("tests/data/stage1/valid/valid.aux");auto&t=db.cells[db.cell_name_to_id.at("terminal_cell")];auto&f=db.cells[db.cell_name_to_id.at("fixed_cell")];auto&ni=db.cells[db.cell_name_to_id.at("fixed_ni_cell")];auto&m=db.cells[db.cell_name_to_id.at("movable_cell")];CHECK(t.terminal&&t.fixed);CHECK_EQ(t.orientation,"FN");CHECK(!f.terminal&&f.fixed);CHECK_EQ(f.orientation,"S");CHECK(!ni.terminal&&ni.fixed);CHECK_EQ(ni.orientation,"W");CHECK(!m.terminal&&!m.fixed);CHECK_EQ(m.orientation,"N");return 0;}

// Consolidated from test_pin_offsets.cpp
#include "placer/io/BookshelfReader.hpp"
#include "../TestSupport.hpp"
int consolidated_bookshelf_6(){auto db=placer::loadBookshelf("tests/data/stage1/valid/valid.aux");CHECK_EQ(db.pins.size(),4u);CHECK_NEAR(db.pins[0].offset_x,1.25,1e-12);CHECK_NEAR(db.pins[0].offset_y,-2.5,1e-12);CHECK_EQ(db.pins[0].direction,"O");CHECK_NEAR(db.pins[1].offset_x,-0.75,1e-12);CHECK_NEAR(db.pins[1].offset_y,0.0,1e-12);CHECK_EQ(db.pins[1].direction,"I");CHECK_NEAR(db.pins[2].offset_x,0.0,1e-12);CHECK_NEAR(db.pins[2].offset_y,3.5,1e-12);CHECK_EQ(db.pins[2].direction,"B");return 0;}

// Consolidated from test_scl_subrows.cpp
#include "placer/io/BookshelfReader.hpp"
#include "../TestSupport.hpp"
int consolidated_bookshelf_7(){auto db=placer::loadBookshelf("tests/data/stage1/valid/valid.aux");CHECK_EQ(db.rows.size(),2u);CHECK_NEAR(db.rows[0].x_start,0,1e-12);CHECK_NEAR(db.rows[0].x_end,6,1e-12);CHECK_NEAR(db.rows[1].x_start,10,1e-12);CHECK_NEAR(db.rows[1].x_end,14,1e-12);auto r=db.region();CHECK_NEAR(r.xl,0,1e-12);CHECK_NEAR(r.xh,14,1e-12);return 0;}

int main(){
  if (consolidated_bookshelf_0()!=0) return 1;
  if (consolidated_bookshelf_1()!=0) return 1;
  if (consolidated_bookshelf_2()!=0) return 1;
  if (consolidated_bookshelf_3()!=0) return 1;
  if (consolidated_bookshelf_4()!=0) return 1;
  if (consolidated_bookshelf_5()!=0) return 1;
  if (consolidated_bookshelf_6()!=0) return 1;
  if (consolidated_bookshelf_7()!=0) return 1;
  return 0;
}
