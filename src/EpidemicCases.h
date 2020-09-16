#ifndef EPIDEMIC_CASES_H
#define EPIDEMIC_CASES_H

#include <boost/shared_ptr.hpp>

class EpidemicDataSet;

class EpidemicCases
{
public:

    EpidemicCases(boost::shared_ptr<EpidemicDataSet> dataSet);

    void setNumCases(int num);
    void setNodeId(int nodeId);

    EpidemicCases& getCases();

    int num;
    int nodeId;
    std::vector<int> stratificationValues;

private:
    boost::shared_ptr<EpidemicDataSet> dataSet_;
};

#endif
