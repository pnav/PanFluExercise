#include "EpidemicCases.h"
#include "EpidemicDataSet.h"

EpidemicCases::EpidemicCases(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    this->dataSet_ = dataSet;

    // add stratification choices
    std::vector<std::vector<std::string> > stratifications = EpidemicDataSet::getStratifications();
    if (stratifications.size() > 0)
    {
        this->stratificationValues.push_back(1); // initial strat set to second index in Widget init
	    for (unsigned int i=1; i < stratifications.size(); ++i)
	    {
            this->stratificationValues.push_back(0); // other strats set to first index (presumably)
	    }
	}
}

void EpidemicCases::setNumCases(int num)
{
    this->num = num;
}

void EpidemicCases::setNodeId(int nodeId)
{
    this->nodeId = nodeId;
}

EpidemicCases& EpidemicCases::getCases()
{
    return *this;
}
