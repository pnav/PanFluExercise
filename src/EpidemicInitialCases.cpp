#include "EpidemicInitialCases.h"
#include "EpidemicDataSet.h"
#include "EpidemicSimulation.h"
#include "EpidemicCases.h"
#include "log.h"
#include <QtXmlPatterns>

EpidemicInitialCases::~EpidemicInitialCases()
{
    clearCases();
}

void EpidemicInitialCases::applyCases()
{
    boost::shared_ptr<EpidemicSimulation> simulation = boost::dynamic_pointer_cast<EpidemicSimulation>(dataSet_);

    if(simulation != NULL)
    {
        if(simulation->getNumTimes() == 1)
        {
            for(unsigned int i=0; i<cases_.size(); i++)
            {
                EpidemicCases cases = cases_[i]->getCases();

                put_flog(LOG_DEBUG, "exposing %i people in %i", cases.num, cases.nodeId);

                simulation->expose(cases.num, cases.nodeId, cases.stratificationValues);
            }
        }
        else
        {
            put_flog(LOG_ERROR, "cannot add initial cases after initial time");
        }
    }
    else
    {
        put_flog(LOG_ERROR, "not a valid simulation");
    }
}


void EpidemicInitialCases::setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    dataSet_ = dataSet;

    // see if this is a simulation
    boost::shared_ptr<EpidemicSimulation> simulation = boost::dynamic_pointer_cast<EpidemicSimulation>(dataSet_);

    // create defaults if this is a simulation
    if(simulation != NULL && simulation->getNumTimes() == 1)
    {
        // todo: this should be handled somewhere else
        int defaultNumCases = 10000;
        std::vector<int> defaultNodeIds;

        defaultNodeIds.push_back(453);
        defaultNodeIds.push_back(113);
        defaultNodeIds.push_back(201);
        defaultNodeIds.push_back(141);
        defaultNodeIds.push_back(375);

        for(unsigned int i=0; i<defaultNodeIds.size(); i++)
        {
            EpidemicCases * cases = new EpidemicCases(simulation);
            cases_.push_back(cases);
            cases->setNumCases(defaultNumCases);
            cases->setNodeId(defaultNodeIds[i]);
        }
    }
}

void EpidemicInitialCases::loadXmlData(const std::string &filename)
{
    // see if this is a simulation
    boost::shared_ptr<EpidemicSimulation> simulation = boost::dynamic_pointer_cast<EpidemicSimulation>(dataSet_);

    if(simulation == NULL)
    {
        put_flog(LOG_ERROR, "ERROR: No valid simulation for EpidemicInitialCases::loadXmlData")
        return;
    }
    else if(simulation->getNumTimes() != 1)
    {
        put_flog(LOG_ERROR, "ERROR: Can only load initial cases at the beginning of the simulation.");
        return;
    }

    QXmlQuery query;

    if(query.setFocus(QUrl(filename.c_str())) == false)
    {
        put_flog(LOG_ERROR, "failed to load %s", filename.c_str());
        return;
    }

    // temp strings
    char string[1024];
    QString qstring;

    // get number of initial cases
    sprintf(string, "string(count(//cases))");
    query.setQuery(string);
    query.evaluateTo(&qstring);
    int numCases = qstring.toInt();

    put_flog(LOG_INFO, "%i entries", numCases);

    // populate parameters for each tile
    for(int i=1; i<=numCases; i++)
    {
        sprintf(string, "string(//cases[%i]/@num)", i);
        query.setQuery(string);
        query.evaluateTo(&qstring);
        int num = qstring.toInt();

        sprintf(string, "string(//cases[%i]/@nodeId)", i);
        query.setQuery(string);
        query.evaluateTo(&qstring);
        int nodeId = qstring.toInt();

        put_flog(LOG_INFO, "%i cases for nodeId %i", num, nodeId);

        EpidemicCases * cases = new EpidemicCases(simulation);
        cases_.push_back(cases);
        cases->setNumCases(num);
        cases->setNodeId(nodeId);
    }
}

void EpidemicInitialCases::clearCases()
{
    for (int i=0; i < cases_.size(); ++i)
    {
        delete cases_[i];
    }
    cases_.clear();
}
