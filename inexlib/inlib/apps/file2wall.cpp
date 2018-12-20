// Copyright (C) 2010, Guy Barrand. All rights reserved.
// See the file inlib.license for terms.

// cowork with wall/mgr/LHCb2wall to send LHCb events.

//inlib_build_use socket

#ifdef INLIB_MEM
#include <inlib/mem>
#endif //INLIB_MEM

#include <inlib/net/sg_client>
#include <inlib/wall/strings>

#include <inlib/sys/atime>
#include <inlib/file>
#include <inlib/sg/strings>
#include <inlib/sg/gui_params>
#include <inlib/sys/sleep>

#include <inlib/args>
#include <iostream>

inline bool send_file(const std::string& a_file,
                      inlib::net::sg_client& a_dc,
                      const inlib::args& a_opts,
                      bool a_verbose) {
  if(a_verbose) std::cout << "send file " << a_file << " ..." << std::endl;

  inlib::atime begin = inlib::atime::now();

  char* uc_buffer;
  long uc_length;
  if(!inlib::file::read_bytes(a_file,uc_buffer,uc_length)) {
    std::cout << "can't read file " << a_file << std::endl;
  } else {
    if(!a_dc.send_buffer_doc(uc_length,uc_length,uc_buffer,a_opts)) {
      std::cout << "send_buffer_doc failed." << std::endl;
      return false;
    }
    delete [] uc_buffer;
  }

  if(a_verbose) {
    std::cout << "file (" << uc_length << " bytes) read+sent done in "
              << inlib::atime::elapsed(begin)
              << std::endl;
  }

  return true;
}

int main(int argc,char** argv) {

#ifdef INLIB_MEM
  inlib::mem::set_check_by_class(true);{
#endif //INLIB_MEM
  inlib::args args(argc,argv);

  //////////////////////////////////////////////////////////
  /// args /////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  bool verbose = args.is_arg("-verbose");

  std::vector<std::string> files;
  args.files(files);
  if(files.empty()) {
    std::cout << "give files to send." << std::endl;
    return EXIT_FAILURE;
  }

  std::string host;
  if(!args.find("-host",host)) {
    std::cout << "argument -host=<string> lacking." << std::endl;
    return EXIT_FAILURE;
  }

  unsigned int port;
  if(!args.find("-port",port)) port = 50800;

  std::string placement;
  if(!args.find("-placement",placement)) {
    std::cout << "argument -placement=[static,dynamic] lacking." << std::endl;
    return EXIT_FAILURE;
  }
  if((placement!=inlib::wall::s_static())&&
     (placement!=inlib::wall::s_dynamic())){
    std::cout << "placement not static or dynamic." << std::endl;
    return EXIT_FAILURE;
  }

  //////////////////////////////////////////////////////////
  /// connect to wall : ////////////////////////////////////
  //////////////////////////////////////////////////////////
  inlib::net::sg_client dc(std::cout,false,true);

  if(verbose)
  std::cout << "try to connected to"
            << " " << host
            << " " << port
            << "..." << std::endl;

  if(!dc.initialize(host,port)) {
    std::cout << "can't connect to"
              << " " << host
              << " " << port
              << "." << std::endl;
    return EXIT_FAILURE;
  }

  if(verbose)
  std::cout << "connected to"
            << " " << host
            << " " << port
            << "." << std::endl;

  //::printf("debug : scene_radius %g\n",scene_radius);
  inlib::args opts;

  opts.add(inlib::wall::s_placement(),placement);

 {float radius;
  if(args.find("-radius",radius)) {
    if(radius<=0) {
      std::cout << "radius <=0" << std::endl;
      return EXIT_FAILURE;
    }
    std::string stmp;
    if(!inlib::num2s(radius,stmp)) {}
    opts.add(inlib::wall::s_radius(),stmp);
  }}

 //if(args.is_arg("-clear")) opts.add(inlib::sg::s_clear_scene());

  //////////////////////////////////////////////////////////
  /// send files : /////////////////////////////////////////
  //////////////////////////////////////////////////////////

  unsigned int _sleep;
  if(!args.find("-sleep",_sleep)) _sleep = 0;

  unsigned int loop;
  if(args.find("-loop",loop)) {
    for(unsigned int index=0;index<loop;index++) {
      inlib_vforcit(std::string,files,it) {
        if(!send_file(*it,dc,opts,verbose)) {}
        if(_sleep) inlib::sleep_secs(_sleep);
      }
    }
  } else {
    while(true) {
      inlib_vforcit(std::string,files,it) {
        if(!send_file(*it,dc,opts,verbose)) {}
        if(_sleep) inlib::sleep_secs(_sleep);
      }
    }
  }

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

 {std::vector<std::string> params;
  inlib::sg::gui_params::names(params);
  std::string cam_prot;
  std::string prot;
  std::vector<std::string>::const_iterator it;
  for(it=params.begin();it!=params.end();++it) {
    std::string s;
    if(args.find("-cam_"+(*it),s)) {
      cam_prot += "\n";
      cam_prot += (*it)+"="+s;
    } else if(args.find("-"+(*it),s)) {
      prot += "\n";
      prot += (*it)+"="+s;
    }
  }
  if(cam_prot.size()) {
    if(verbose) {
      std::cout << "send set_camera :" << std::endl << cam_prot << std::endl;
    }
    if(!dc.socket().send_string(inlib::wall::protocol::s_rwc_set_camera()+cam_prot)) {}
  }
  if(prot.size()) {
    if(verbose) {
      std::cout << "send set_params :" << std::endl << prot << std::endl;
    }
    if(!dc.socket().send_string(inlib::wall::protocol::s_rwc_set_params()+prot)) {}
  }}

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

  if(!dc.socket().send_string(inlib::wall::protocol::s_disconnect())) {}
  dc.socket().disconnect();

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////

#ifdef INLIB_MEM
  }inlib::mem::balance(std::cout);
#endif //INLIB_MEM

  return EXIT_SUCCESS;
}
