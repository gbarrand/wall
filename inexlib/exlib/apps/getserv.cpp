// Copyright (C) 2010, Guy Barrand. All rights reserved.
// See the file exlib.license for terms.

//exlib_build_use inlib tntnet zip zlib thread dl

//#define EXLIB_SRV_DEBUG

#ifdef _WIN32
#include <iostream>
int main() {
  std::cout << "application not yet ported on Windows." << std::endl;
  return 1;
}

#else //_WIN32

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#include <inlib/cmd/rest>
#include <inlib/cmd/cached>

#include <inlib/astro>

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#include <tnt/ecpp.h>
#include <tnt/componentfactory.h>
#include <tnt/httpreply.h>
#include <tnt/tntnet.h> //for shutdown.

INLIB_GLOBAL_STRING(exlib_app_args)

namespace getserv {

class component : public tnt::EcppComponent {

  INLIB_CLASS_STRING(cmd)

  INLIB_CLASS_STRING_VALUE(cmd_get_from_home,get_from_home)
  INLIB_CLASS_STRING_VALUE(cmd_stripe82_get,stripe82_get) //backcomp

public:
  component(const tnt::Compident& a_ci,const tnt::Urlmapper& a_um,tnt::Comploader& a_cl)
  :EcppComponent(a_ci,a_um,a_cl)
  {
    //::printf("debug : create component\n");
    // wall/master_begin.icc :
    //   UPDATE_NODE_cache.set_size(4*screens.size()); //4 image per screen.
    m_POST_cache.set_size(4*12); //LAL wall : 12*4=48

  }

protected:
  ~component(){
    //::printf("debug : delete component\n");
  }

protected:
  inlib::cmd::session* get_session(tnt::HttpRequest& a_request) {
    tnt::Scope& _scope = a_request.getSessionScope();

    //TNT_SESSION_COMPONENT_VAR(session,s_session,"session s_session",());
    const std::string key = getComponentScopePrefix(getCompident())+"::session";
    inlib::cmd::session* _session = _scope.get<inlib::cmd::session>(key);
    if(!_session) {
      std::string sargs;
      std::string app_name;
      inlib_mforcit(std::string,std::string,a_request.getArgs(),it) {
        if((*it).first==s_exlib_app_args()) sargs = (*it).second;
      }
      inlib::args args(sargs," ",true);
      
      _session = new inlib::cmd::session(args);
      if(!_session || _session->pwd().empty()) {
        delete _session;
        return 0;
      }

      _session->add_cmd(s_cmd_get_from_home(),new inlib::cmd::cached_get_cmd<cxxtools::Mutex>(m_POST_cache));

      //backcomp
      _session->add_cmd(s_cmd_stripe82_get(),new inlib::cmd::cached_get_cmd<cxxtools::Mutex>(m_POST_cache));

      _scope.put<inlib::cmd::session>(key,_session);
    }
    //::printf("debug : scopekey : %s : %lu\n",key.c_str(),_session);

    return _session;
  }
  void delete_session(tnt::HttpRequest& a_request) {
    tnt::Scope& _scope = a_request.getSessionScope();
    const std::string key = getComponentScopePrefix(getCompident())+"::session";
    _scope.erase(key); //it deletes the session object.
  }
public:
  unsigned operator() (tnt::HttpRequest& a_request,tnt::HttpReply& a_reply,tnt::QueryParams& a_qparam) {

    inlib::cmd::session* psession = get_session(a_request);
    if(!psession) {
      std::string msg = " new session() failed, home directory not found.";
      a_reply.sout() << inlib::cmd::s_failed() << " : " << msg;
      return HTTP_OK;
    }
    inlib::cmd::session& _session = *psession;

    //std::string cmd = a_qparam.getUrl();
    //::printf("debug : a_qparam.getUrl() : \"%s\"\n",a_qparam.getUrl().c_str());

    std::string cmd = a_qparam.param(s_cmd());
    if(_session.verbose()) ::printf("getserv : cmd : \"%s\"\n",cmd.c_str());

    std::string rest;
    inlib::cmd::rest(cmd,rest);

    if(inlib::cmd::base_cmd* _cmd = _session.find_cmd(cmd)) {
      std::vector<std::string> words;
      inlib::words(rest," ",false,words);
      _cmd->exec(_session,words,a_reply.sout(),a_reply.out());
    } else {
      _session.reply_failed(a_reply.sout(),"unknown command.");
    }

    return HTTP_OK;
  }
protected:
  inlib::buffer_cache<cxxtools::Mutex> m_POST_cache;
};

}

//////////////////////////////////////////////////
/// main : ////////////////////////////////////////
//////////////////////////////////////////////////

#include <tnt/tntnet.h>

#ifdef INLIB_MEM
#include <inlib/mem>
#endif

#include <inlib/net/base_socket>
#include <inlib/sargs>

#include <cstdlib>

int main(int argc,char* argv[]){
#ifdef INLIB_MEM
  inlib::mem::set_check_by_class(true);{
#endif

  inlib::args args(argc,argv);

  if(args.is_arg(inlib::s_arg_help())) {
    std::cout << "args :" << std::endl
              << inlib::s_arg_help() << std::endl
              << inlib::s_arg_verbose() << std::endl
              << inlib::s_arg_host() << std::endl
              << inlib::s_arg_port() << std::endl
              << std::endl;
    return EXIT_SUCCESS;
  }

  bool verbose = args.is_arg(inlib::s_arg_verbose());

  std::string host;
  args.find(inlib::s_arg_host(),host,"0.0.0.0");
  unsigned int port;
  args.find<unsigned int>(inlib::s_arg_port(),port,inlib::proxy_port());
  if(verbose) {
    std::cout << "host " << host << std::endl;
    std::cout << "port " << port << std::endl;
  }

  tnt::ComponentFactoryImpl<getserv::component> Factory("getserv");

  try  { 

    tnt::Tntnet app;    

    app.setAppName("getserv");

    app.listen(host,port);    

    //NOTE : tnt example is "^/$", but the below pattern is needed for web clients.
    tnt::Mapping& murl = app.mapUrl("^/(.*)$","getserv");
    murl.setPathInfo("/getserv");

   {std::map<std::string,std::string> _args;
    std::string sargs; //space separated args : "-key1=value1 -key2=value2 ..."
    std::vector<std::string> v;args.to_vector(v);
    inlib_vforcit(std::string,v,it) {
      sargs += " ";
      sargs += *it;
    }
    _args[s_exlib_app_args()] =  sargs;
    murl.setArgs(_args);}

    if(verbose) std::cout << "tnt::Tntnet.run() ..." << std::endl;
    app.run(); 

  }  catch (const std::exception& a_e)  {
    std::cerr << a_e.what() << std::endl;
  }

#ifdef INLIB_MEM
  }inlib::mem::balance(std::cout);
  std::cout << "getserv : (mem) exit..." << std::endl;
#else
  std::cout << "getserv : exit..." << std::endl;
#endif

  return EXIT_SUCCESS;
}

#endif //_WIN32
