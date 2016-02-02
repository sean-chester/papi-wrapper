/**
 * A minimum working example (mwe) to illustrate the use of the papi-wrapper library. 
 * To get started quickly, just replace the code that sums random numbers in a parallel 
 * loop (lines 96-104) with statements that are more meaningful to you.
 */


#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <sstream>	// for std::istream
#include <iterator>	// for std::istream_iterator
#include <getopt.h>
#include <cstring> // for strcmp


#include "papi_wrapper.h"


/**
 * Executes code that should be done prior to any papi tracking (in this case, 
 * babbling to stdout).
 */
void code_init() {

	std::cout << "This is where you do stuff that should not be tracked by papi." << std::endl;
}

/**
 * Initialises papi for use on each thread.
 */
void papi_init( uint32_t num_threads ) {

	srand( (unsigned) time(0) );
	
	if( PAPI_is_initialized() == PAPI_NOT_INITED ) {
	
		// Initialize PAPI library for each thread.
		PAPI_library_init( PAPI_VER_CURRENT );
#pragma omp parallel num_threads( num_threads )
		{
			if( PAPI_thread_init( pthread_self ) != PAPI_OK ) {
				 exit(0);
			}
		}
	}
}


/**
 * A sample papi-tracked routine that just sums a bunch of random numbers.
 * @param num_threads The number of threads that will be active in the routine
 * @param papi_mode The type of papi counter to track (see papi_base for a list of 
 * possible values).
 * @param custom_events An set of custom events: only used if in papi_mode_custom.
 */
void execute( const uint32_t num_threads, const uint32_t papi_mode, 
	std::vector< std::pair< std::string, std::string > > &custom_events ) {

	/* Declare an array of polymorphic papi counter sets, one for each thread */
	std::vector< papi_base* > papi_counters;
	

	/* Initialise and start the papi counter sets, one for each thread */
	if( papi_mode != papi_base::papi_mode_off ) {
		
		for( uint32_t i = 0; i < num_threads; ++i ) {
	
			switch ( papi_mode ) {
	
				case papi_base::papi_mode_custom :
					papi_counters.push_back( new papi_custom( custom_events ) );
					break;
		
				case papi_base::papi_mode_cache :
					papi_counters.push_back( new papi_cache() );
					break;
		
				case papi_base::papi_mode_branch :
					papi_counters.push_back( new papi_branch() );
					break;
		
				case papi_base::papi_mode_throughput :
					papi_counters.push_back( new papi_cycles() );
					break;
		
				case papi_base::papi_mode_tlb :
					papi_counters.push_back( new papi_tlb() );
					break;
			}
		}
		papi_base::start_papi_array( papi_counters );
	}

	
	/* Do some mind-bending computation (e.g., sum random integers in parallel) */
	const uint32_t n = 100000000;
	uint32_t sum = 0;
	
	#pragma omp parallel for reduction( +:sum ) num_threads( num_threads )
	for( uint32_t i = 0; i < n; ++i ) {
		sum += rand();
	}
	std::cout << "random sum = " << sum << std::endl;
	
	
	
	/* Report the PAPI counters' values, summed over all threads. */
	if( papi_mode != papi_base::papi_mode_off ) {
		
		papi_base::stop_papi_array( papi_counters ); 
		papi_base::sum_papi_array( papi_counters );
		std::cout << *( papi_counters[ 0 ] ) << std::endl;
		
	}
	
	/* clean up */
	while( papi_counters.size() > 0 ) {
		papi_counters.pop_back();
	}

	return;
}

/** Echoes out usage instructions to the user */
void printUsage() {
	using namespace std;
  cout << endl;
  cout << "papi-wrapper-mwe - a wrapper for easy multicore papi reporting" << endl 
  << endl;
  cout << "USAGE: ./papi-wrapper-mwe [-t num_threads] [-p papi_mode] [-c custom_events]" << endl;
  cout << " -t: num_threads with which to run (default 4)" << endl;
  cout << " -p: papi mode in which to run (default: \"none\")" << endl;
  cout << "     Supported prebuilt PAPI modes: " 
  	<< "branch, throughput, cache, tlb; or run in mode \"custom\"" << endl;
  cout << " -c: custom papi counters (either present events or native events); " 
   << "has no effect if -p is not \"custom\"." << endl;
  cout << "     Example: -c \"MISALIGN_MEM_REF:LOADS MISALIGN_MEM_REF:STORES\"" << endl 
  << endl;
  cout << "Example: ./papi-wrapper-mew -t 4 -p \"custom\" -c \"CPU_CLK_UNHALTED\"" << endl
  << endl;
}


/** Handles the input arguments given by the user and returns 0 upon success */
int handleArguments( int argc, char** argv, uint32_t *t, uint32_t *mode, 
	std::vector< std::pair< std::string, std::string > > &custom ) {

  int c;
  opterr = 0;
  
  while( ( c = getopt( argc, argv, "t:p:c:" ) ) != -1 ) {
    switch (c) {
    case 't':
      *t = atoi( optarg );
      break;
    case 'p':
      if( std::strcmp( optarg, "branch" ) == 0 ) { *mode = papi_base::papi_mode_branch; }
      if( std::strcmp( optarg, "tlb" ) == 0 ) { *mode = papi_base::papi_mode_tlb; }
      if( std::strcmp( optarg, "throughput" ) == 0 ) { *mode = papi_base::papi_mode_throughput; }
      if( std::strcmp( optarg, "cache" ) == 0 ) { *mode = papi_base::papi_mode_cache; }
      if( std::strcmp( optarg, "custom" ) == 0 ) { *mode = papi_base::papi_mode_custom; }
      break;
    case 'c':
    	{
				// [Tokenise a string to a vector](http://stackoverflow.com/a/5607650/2769271)
				std::string s = std::string( optarg );
				std::stringstream ss( s );
				std::istream_iterator<std::string> begin(ss);
				std::istream_iterator<std::string> end;
				std::vector< std::string > custom_strings;
				std::copy( begin, end, std::back_inserter< std::vector< std::string > > ( custom_strings ) );
				for( auto it = custom_strings.begin(); it != custom_strings.end(); ++it ) {
				
				//transform vector to pairs.
				custom.push_back( std::pair< std::string, std::string >( *it, *it ) );
				}
				break;
      }
    default:
      if( isprint( optopt ) ) { 
        std::cerr << "Unknown option `-" << optopt << "'." << std::endl;
      }
      printUsage();
      return 1;
    }
  }

  if( argc == 1 || optind != argc ) {
    printUsage();
    return 1;
  } 
  return 0; 
}


int main( int argc, char** argv ) {

  uint32_t num_threads = 4;
  uint32_t papi_mode = papi_base::papi_mode_off;
  std::vector< std::pair< std::string, std::string > > custom;

	int err = handleArguments( argc, argv, &num_threads, &papi_mode, custom );
	if( err != 0 ) { return err; }

  code_init();
  papi_init( num_threads );
  execute( num_threads, papi_mode, custom );

  return 0;
}
