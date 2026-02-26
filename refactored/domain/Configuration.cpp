#include "Configuration.h"

namespace Domain {

Configuration::Configuration()
    : m_isStartEnabled(false)
    , m_autoSave(false)
    , m_isRational(false)
    , m_isCreated(false)
    , m_stopContinueState(0)
    , m_operationMode(0)
    , m_exponentType(0)
    , m_savedChromosomeCount(0)
    , m_saveLoadedData(0)
    , m_cycleCount(0)
    , m_sse(0.0)
    , m_jnrr(0.0)
    , m_iterations(0)
    , m_previousIterations(0)
{
}

void Configuration::reset()
{
    m_isStartEnabled = false;
    m_autoSave = false;
    m_isRational = false;
    m_isCreated = false;
    m_stopContinueState = 0;
    m_operationMode = 0;
    m_exponentType = 0;
    m_savedChromosomeCount = 0;
    m_saveLoadedData = 0;
    m_cycleCount = 0;
    m_sse = 0.0;
    m_jnrr = 0.0;
    m_iterations = 0;
    m_previousIterations = 0;
    
    m_population.clear();
    m_bestPreviousFitness.clear();
    m_decimation.clear();
    m_decimationSize.clear();
    m_configFileName.clear();
}

bool Configuration::isValid() const
{
    return m_isCreated && 
           m_algorithmData.variables.isValid() &&
           m_algorithmData.populationSize > 0;
}

} // namespace Domain
