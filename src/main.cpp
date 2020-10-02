#include "main.h"
#include "log.h"
#include <cstdlib>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

#include "models/disease/StochasticSEATIRD.h"
#include "Parameters.h"
#include "EpidemicInitialCases.h"

#define BATCH_TIMESTEPS 1
#define BATCH_INITIAL_CASES_FILENAME 2
#define BATCH_PARAM_FILENAME 4
#define BATCH_OUTPUT_VAR 8
#define BATCH_OUTPUT_FILENAME 16
#define BATCH_ALL_PARAMS 31

bool g_batchMode = true;
int g_batchNumTimesteps = 240;
std::string g_batchInitialCasesFilename;
std::string g_batchParametersFilename;
std::string g_batchOutputVariable = "treatable";
std::string g_batchOutputFilename = "treatable.csv";

std::string g_dataDirectory;

int main(int argc, char * argv[])
{
    // declare the supported options
    boost::program_options::options_description programOptions("Allowed options");

    programOptions.add_options()
        ("help", "produce help message")
        ("batch-numtimesteps", boost::program_options::value<int>(), "limit batch run to <n> time steps")
        ("batch-initialcasesfilename", boost::program_options::value<std::string>(), "batch mode initial cases filename")
        ("batch-parametersfilename", boost::program_options::value<std::string>(), "batch mode parameters filename")
        ("batch-outputvariable", boost::program_options::value<std::string>(), "batch output variable")
        ("batch-outputfilename", boost::program_options::value<std::string>(), "batch output filename")
    ;

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, programOptions), vm);
    boost::program_options::notify(vm);
    put_flog(LOG_INFO, "enabling batch mode");

    if(vm.count("help"))
    {
        std::cout << programOptions << std::endl;
        return 1;
    }

    unsigned int params_found = 0;

    if(vm.count("batch-numtimesteps"))
    {
        g_batchNumTimesteps = vm["batch-numtimesteps"].as<int>();
        put_flog(LOG_INFO, "got batch num time steps %i", g_batchNumTimesteps);
        params_found = params_found | BATCH_TIMESTEPS;
    }

    if(vm.count("batch-initialcasesfilename"))
    {
        g_batchInitialCasesFilename = vm["batch-initialcasesfilename"].as<std::string>();
        put_flog(LOG_INFO, "got batch initial cases filename %s", g_batchInitialCasesFilename.c_str());
        params_found = params_found | BATCH_INITIAL_CASES_FILENAME;
    }

    if(vm.count("batch-parametersfilename"))
    {
        g_batchParametersFilename = vm["batch-parametersfilename"].as<std::string>();
        put_flog(LOG_INFO, "got batch parameters filename %s", g_batchParametersFilename.c_str());
        params_found = params_found | BATCH_PARAM_FILENAME;
    }

    if(vm.count("batch-outputvariable"))
    {
        g_batchOutputVariable = vm["batch-outputvariable"].as<std::string>();
        put_flog(LOG_INFO, "got batch output variable %s", g_batchOutputVariable.c_str());
        params_found = params_found | BATCH_OUTPUT_VAR;
    }

    if(vm.count("batch-outputfilename"))
    {
        g_batchOutputFilename = vm["batch-outputfilename"].as<std::string>();
        put_flog(LOG_INFO, "got batch output filename %s", g_batchOutputFilename.c_str());
        params_found = params_found | BATCH_OUTPUT_FILENAME;
    }

    // end argument parsing

    if (params_found < BATCH_ALL_PARAMS)
    {
        if (!(params_found & BATCH_TIMESTEPS)) put_flog(LOG_FATAL, "missing batch-numtimesteps parameter");
        if (!(params_found & BATCH_INITIAL_CASES_FILENAME)) put_flog(LOG_FATAL, "missing batch-initialcasesfilename parameter");
        if (!(params_found & BATCH_PARAM_FILENAME)) put_flog(LOG_FATAL, "missing batch-parametersfilename parameter");
        if (!(params_found & BATCH_OUTPUT_VAR)) put_flog(LOG_FATAL, "missing batch-outputvariable parameter");
        if (!(params_found & BATCH_OUTPUT_FILENAME)) put_flog(LOG_FATAL, "missing batch-outputfilename parameter");
        exit(1);
    }

    // assumes data directory is in a sister directory to the current executable    
    g_dataDirectory = std::string(getenv("PANFLU_DATA"));

    put_flog(LOG_DEBUG, "data directory: %s", g_dataDirectory.c_str());

    put_flog(LOG_INFO, "starting batch mode");

    // create a new simulation using the StochasticSEATIRD model
    boost::shared_ptr<EpidemicSimulation> simulation(new StochasticSEATIRD());

    EpidemicInitialCases *initialCases_ = new EpidemicInitialCases();
    initialCases_->setDataSet(simulation);

    if(g_batchInitialCasesFilename.empty() != true)
    {
        initialCases_->loadXmlData(g_batchInitialCasesFilename);
    }

    if(g_batchParametersFilename.empty() != true)
    {
        // defined in Parameters.h/.cpp
        g_parameters.loadXmlData(g_batchParametersFilename);
    }

    for(int i=0; i<g_batchNumTimesteps; i++)
    {
        if(i >= simulation->getNumTimes())
        {
            // if this is the first time simulated, set the initial cases
            if(i == 1)
            {
                initialCases_->applyCases();
            }

            simulation->simulate();
        }        
    }

    std::string out = simulation->getVariableStratified2NodeVsTime(g_batchOutputVariable);

    {
        std::ofstream ofs(g_batchOutputFilename.c_str());
        ofs << out;
    }

    put_flog(LOG_INFO, "done with batch mode");

    // clean up
    delete initialCases_;

    return 0;
}


