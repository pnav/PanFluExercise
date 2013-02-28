#include "StochasticSEATIRD.h"
#include "../../Parameters.h"
#include "../random.h"
#include "../../log.h"
#include <boost/bind.hpp>

StochasticSEATIRD::StochasticSEATIRD()
{
    put_flog(LOG_DEBUG, "");

    // create other required variables for this model
    newVariable("asymptomatic");
    newVariable("treatable");
    newVariable("infectious");
    newVariable("recovered");
    newVariable("deceased");

    // derived variables
    derivedVariables_[":infected"] = boost::bind(&StochasticSEATIRD::getInfected, this, _1, _2, _3);
    derivedVariables_[":hospitalized"] = boost::bind(&StochasticSEATIRD::getHospitalized, this, _1, _2, _3);

    // initial start time to 0
    now_ = 0.;

    // initiate random number generator
    gsl_rng_env_setup();
    randGenerator_ = gsl_rng_alloc (gsl_rng_default);
}

StochasticSEATIRD::~StochasticSEATIRD()
{
    put_flog(LOG_DEBUG, "");

    gsl_rng_free(randGenerator_);
}

int StochasticSEATIRD::expose(int num, int nodeId, std::vector<int> stratificationValues)
{
    int numExposed = EpidemicSimulation::expose(num, nodeId, stratificationValues);

    // create events based on these new exposures
    for(int i=0; i<numExposed; i++)
    {
        StochasticSEATIRDSchedule schedule(now_, rand_, stratificationValues);

        initializeExposedTransitions(nodeId, stratificationValues, schedule);
        initializeContactEvents(nodeId, stratificationValues, schedule);
    }

    return numExposed;
}

void StochasticSEATIRD::simulate()
{
    EpidemicSimulation::simulate();

    double tMax = now_ + 1.0;

    // process events for each node
    for(unsigned int i=0; i<nodeIds_.size(); i++)
    {
        int nodeId = nodeIds_[i];

        while(!eventQueue_[nodeId].empty() && eventQueue_[nodeId].top().time < tMax)
        {
            nextEvent(nodeId);
        }
    }

    // travel between nodes
    travel();

    now_ = tMax;
}

float StochasticSEATIRD::getInfected(int time, int nodeId, std::vector<int> stratificationValues)
{
    float infected = 0.;
    infected += getValue("asymptomatic", time, nodeId, stratificationValues);
    infected += getValue("treatable", time, nodeId, stratificationValues);
    infected += getValue("infectious", time, nodeId, stratificationValues);

    return infected;
}

float StochasticSEATIRD::getHospitalized(int time, int nodeId, std::vector<int> stratificationValues)
{
    float hospitalized = 0.;
    hospitalized += getValue("treatable", time, nodeId, stratificationValues);
    hospitalized += getValue("infectious", time, nodeId, stratificationValues);

    // todo: this is just an example...
    hospitalized *= 0.05;

    return hospitalized;
}

void StochasticSEATIRD::addEvent(const int &nodeId, const StochasticSEATIRDEvent &event)
{
    eventQueue_[nodeId].push(event);

    if(event.fromStratificationValues != event.toStratificationValues)
    {
        // todo: contact counter increment?
    }
}

void StochasticSEATIRD::initializeExposedTransitions(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule)
{
    StochasticSEATIRDEvent event;

    event.initializationTime = now_;
    event.time = schedule.Ta;
    event.type = EtoA;
    event.fromStratificationValues = stratificationValues;
    event.toStratificationValues = stratificationValues;

    addEvent(nodeId, event);

    initializeAsymptomaticTransitions(nodeId, stratificationValues, schedule);
}

void StochasticSEATIRD::initializeAsymptomaticTransitions(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule)
{
    StochasticSEATIRDEvent event;

    event.initializationTime = schedule.Ta;
    event.fromStratificationValues = stratificationValues;
    event.toStratificationValues = stratificationValues;

    // event time and type will vary...
    if(schedule.Tt < schedule.Tr_a && schedule.Tt < schedule.Td_a)
    {
        // asymptomatic -> treatable
        event.time = schedule.Tt;
        event.type = AtoT;

        initializeTreatableTransitions(nodeId, stratificationValues, schedule);
    }
    else if(schedule.Tr_a < schedule.Td_a)
    {
        // asymptomatic -> recovered
        event.time = schedule.Tr_a;
        event.type = AtoR;
    }
    else
    {
        // asymptomatic -> deceased
        event.time = schedule.Td_a;
        event.type = AtoD;
    }

    addEvent(nodeId, event);
}

void StochasticSEATIRD::initializeTreatableTransitions(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule)
{
    StochasticSEATIRDEvent event;

    event.initializationTime = schedule.Tt;
    event.fromStratificationValues = stratificationValues;
    event.toStratificationValues = stratificationValues;

    // event time and type will vary...
    if(schedule.Ti < schedule.Tr_ti && schedule.Ti < schedule.Td_ti)
    {
        // treatable -> infectious
        event.time = schedule.Ti;
        event.type = TtoI;

        initializeInfectiousTransitions(nodeId, stratificationValues, schedule);
    }
    else if(schedule.Tr_ti < schedule.Td_ti)
    {
        // treatable -> recovered
        event.time = schedule.Tr_ti;
        event.type = TtoR;
    }
    else
    {
        // treatable -> deceased
        event.time = schedule.Td_ti;
        event.type = TtoD;
    }

    addEvent(nodeId, event);
}

void StochasticSEATIRD::initializeInfectiousTransitions(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule)
{
    StochasticSEATIRDEvent event;

    event.initializationTime = schedule.Ti;
    event.fromStratificationValues = stratificationValues;
    event.toStratificationValues = stratificationValues;

    // event time and type will vary...
    if(schedule.Tr_ti < schedule.Td_ti)
    {
        // infectious -> recovered
        event.time = schedule.Tr_ti;
        event.type = ItoR;
    }
    else
    {
        // infectious -> deceased
        event.time = schedule.Td_ti;
        event.type = ItoD;
    }

    addEvent(nodeId, event);
}

void StochasticSEATIRD::initializeContactEvents(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule)
{
    // todo: beta should be age-specific considering PHA's
    double beta = g_parameters.getR0() / g_parameters.getBetaScale();

    // todo: need actual value here, should be age-specific
    double vaccineEffectiveness = 0.;

    // todo: should be in parameters
    double sigma[] = {1.00, 0.98, 0.94, 0.91, 0.66};

    // todo: should be in parameters
    double contact[5][5] = {    { 45.1228487783,8.7808312353,11.7757947836,6.10114751268,4.02227175596 },
                                { 8.7808312353,41.2889143668,13.3332813497,7.847051289,4.22656343551 },
                                { 11.7757947836,13.3332813497,21.4270155984,13.7392636644,6.92483172729 },
                                { 6.10114751268,7.847051289,13.7392636644,18.0482119252,9.45371062356 },
                                { 4.02227175596,4.22656343551,6.92483172729,9.45371062356,14.0529294262 }   };

    // make sure we have expected stratifications
    // todo: make these defined elsewhere?
    int numAgeGroups = 5;
    int numRiskGroups = 2;
    int numVaccinatedGroups = 2;

    if((int)stratifications_[0].size() != numAgeGroups || (int)stratifications_[1].size() != numRiskGroups || (int)stratifications_[2].size() != numVaccinatedGroups)
    {
        put_flog(LOG_ERROR, "wrong number of stratifications");
        return;
    }

    for(int a=0; a<numAgeGroups; a++)
    {
        for(int r=0; r<numRiskGroups; r++)
        {
            for(int v=0; v<numVaccinatedGroups; v++)
            {
                std::vector<int> toStratificationValues;
                toStratificationValues.push_back(a);
                toStratificationValues.push_back(r);
                toStratificationValues.push_back(v);

                // current time
                int time = numTimes_ - 1;

                // fraction of the to group in population; this was cached before
                double toGroupFraction = getValue("population", time, nodeId, toStratificationValues) / getValue("population", time, nodeId);

                double contactRate = contact[stratificationValues[0]][a];
                double transmissionRate = (1. - vaccineEffectiveness) * beta * contactRate * sigma[a] * toGroupFraction;
                double TcInit = schedule.Ta;
                double Tc = random_exponential(transmissionRate, &rand_) + TcInit;

                while(Tc < schedule.Trd_ati)
                {
                    StochasticSEATIRDEvent event;

                    event.initializationTime = TcInit;
                    event.time = Tc;
                    event.type = CONTACT;
                    event.fromStratificationValues = stratificationValues;
                    event.toStratificationValues = toStratificationValues;

                    addEvent(nodeId, event);

                    TcInit = Tc;
                    Tc = TcInit + random_exponential(transmissionRate, &rand_);
                }
            }
        }
    }
}

bool StochasticSEATIRD::nextEvent(int nodeId)
{
    if(eventQueue_[nodeId].empty() == true)
    {
        return false;
    }

    // get and pop first event
    StochasticSEATIRDEvent event = eventQueue_[nodeId].top();
    eventQueue_[nodeId].pop();

    now_ = event.time;

    switch(event.type)
    {
        case EtoA:
            // exposed -> asymptomatic
            transition(1, "exposed", "asymptomatic", nodeId, event.fromStratificationValues);
            break;

        case AtoT:
            // asymptomatic -> treatable
            transition(1, "asymptomatic", "treatable", nodeId, event.fromStratificationValues);
            break;
        case AtoR:
            // asymptomatic -> recovered
            transition(1, "asymptomatic", "recovered", nodeId, event.fromStratificationValues);
            break;
        case AtoD:
            // asymptomatic -> deceased
            transition(1, "asymptomatic", "deceased", nodeId, event.fromStratificationValues);
            break;

        // todo: for TtoI, TtoR, TtoD look at keep_event, unqueue_event...
        case TtoI:
            // treatable -> infectious
            transition(1, "treatable", "infectious", nodeId, event.fromStratificationValues);
            break;
        case TtoR:
            // treatable -> recovered
            transition(1, "treatable", "recovered", nodeId, event.fromStratificationValues);
            break;
        case TtoD:
            // treatable -> deceased
            transition(1, "treatable", "deceased", nodeId, event.fromStratificationValues);
            break;

        // todo: for ItoR, ItoD look at keep_event
        case ItoR:
            // infectious -> recovered
            transition(1, "infectious", "recovered", nodeId, event.fromStratificationValues);
            break;
        case ItoD:
            // infectious -> deceased
            transition(1, "infectious", "deceased", nodeId, event.fromStratificationValues);
            break;

        case CONTACT:
            // todo: see if we keep this contact event
            {
                // current time
                int time = numTimes_ - 1;

                int targetPopulationSize = (int)getValue("population", time, nodeId, event.toStratificationValues);

                if(event.fromStratificationValues == event.toStratificationValues)
                {
                    targetPopulationSize -= 1; // - 1 because randint includes both endpoints
                }

                if(targetPopulationSize > 0)
                {
                    // random integer between 1 and targetPopulationSize
                    int contact = rand_.randInt(targetPopulationSize - 1) + 1;

                    if((int)getValue("susceptible", time, nodeId, event.toStratificationValues) >= contact)
                    {
                        expose(1, nodeId, event.toStratificationValues);
                    }
                }
            }
    }

    return true;
}

void StochasticSEATIRD::travel()
{
    // current time
    int time = numTimes_ - 1;

    int numAgeGroups = 5;
    int numRiskGroups = 2;
    int numVaccinatedGroups = 2;

    // todo: these should be parameters defined elsewhere
    double RHO = 0.39;

    double sigma[] = { 1.00, 0.98, 0.94, 0.91, 0.66 };

    double contact[5][5] = {    { 45.1228487783,8.7808312353,11.7757947836,6.10114751268,4.02227175596 },
                                { 8.7808312353,41.2889143668,13.3332813497,7.847051289,4.22656343551 },
                                { 11.7757947836,13.3332813497,21.4270155984,13.7392636644,6.92483172729 },
                                { 6.10114751268,7.847051289,13.7392636644,18.0482119252,9.45371062356 },
                                { 4.02227175596,4.22656343551,6.92483172729,9.45371062356,14.0529294262 }   };

    double vaccineEffectiveness = 0.8;

    for(unsigned int sinkNodeIndex=0; sinkNodeIndex < nodeIds_.size(); sinkNodeIndex++)
    {
        int sinkNodeId = nodeIds_[sinkNodeIndex];

        std::vector<double> unvaccinatedProbabilities(numAgeGroups, 0.0);

        std::vector<double> ageBasedFlowReductions (5, 1.0);
        ageBasedFlowReductions[0] = 10; // 0-4  year olds
        ageBasedFlowReductions[1] = 2;  // 5-24 year olds
        ageBasedFlowReductions[4] = 2;  // 65+  year olds

        for(unsigned int sourceNodeIndex=0; sourceNodeIndex < nodeIds_.size(); sourceNodeIndex++)
        {
            int sourceNodeId = nodeIds_[sourceNodeIndex];

            if(sinkNodeId != sourceNodeId)
            {
                // flow data
                float travelFractionIJ = getTravel(sinkNodeId, sourceNodeId);
                float travelFractionJI = getTravel(sourceNodeId, sinkNodeId);

                if(travelFractionIJ > 0. || travelFractionJI > 0.)
                {
                    for(int a=0; a<numAgeGroups; a++)
                    {
                        double numberOfInfectiousContactsIJ = 0.;
                        double numberOfInfectiousContactsJI = 0.;

                        // todo: beta should be age-specific considering PHA's
                        double beta = g_parameters.getR0() / g_parameters.getBetaScale();

                        for(int b=0; b<numAgeGroups; b++)
                        {
                            std::vector<int> stratificationAgeB(1, b);

                            double asymptomatic = getValue("asymptomatic", time, sourceNodeId, stratificationAgeB);

                            double transmitting = getValue("asymptomatic", time, sourceNodeId, stratificationAgeB) + getValue("treatable", time, sourceNodeId, stratificationAgeB) + getValue("infectious", time, sourceNodeId, stratificationAgeB);

                            double contactRate = contact[a][b];

                            numberOfInfectiousContactsIJ += transmitting * beta * RHO * contactRate * sigma[a] / ageBasedFlowReductions[a];
                            numberOfInfectiousContactsJI += asymptomatic * beta * RHO * contactRate * sigma[a] / ageBasedFlowReductions[b];
                        }

                        unvaccinatedProbabilities[a] += travelFractionIJ * numberOfInfectiousContactsIJ / getValue("population", time, sourceNodeId);
                        unvaccinatedProbabilities[a] += travelFractionJI * numberOfInfectiousContactsJI / getValue("population", time, sinkNodeId);
                    }
                }
            }
        }

        for(int a=0; a<numAgeGroups; a++)
        {
            for(int r=0; r<numRiskGroups; r++)
            {
                for(int v=0; v<numVaccinatedGroups; v++)
                {
                    double probability = 0.;

                    // vaccined stratification == 1
                    if(v == 1)
                    {
                        probability = (1. - vaccineEffectiveness) * unvaccinatedProbabilities[a];
                    }
                    else
                    {
                        probability = unvaccinatedProbabilities[a];
                    }

                    std::vector<int> stratificationValues;
                    stratificationValues.push_back(a);
                    stratificationValues.push_back(r);
                    stratificationValues.push_back(v);

                    int sinkNumSusceptible = (int)(getValue("susceptible", time, sinkNodeId, stratificationValues) + 0.5); // continuity correction

                    double numberOfExposures = gsl_ran_binomial(randGenerator_, probability, sinkNumSusceptible);

                    expose(numberOfExposures, sinkNodeId, stratificationValues);
                }
            }
        }
    }
}
