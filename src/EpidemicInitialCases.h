#ifndef EPIDEMIC_INITIAL_CASES_H
#define EPIDEMIC_INITIAL_CASES_H

#include <boost/shared_ptr.hpp>

class EpidemicDataSet;
class EpidemicCases;

class EpidemicInitialCases
{
public:
    EpidemicInitialCases() {}
    ~EpidemicInitialCases();

	void applyCases();
    void setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet);
    void loadXmlData(const std::string &filename);

private:
    void clearCases();

    boost::shared_ptr<EpidemicDataSet> dataSet_;
    std::vector<EpidemicCases *> cases_;
};

#endif