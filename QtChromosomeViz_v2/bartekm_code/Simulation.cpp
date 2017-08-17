#include "Simulation.h"

#include <QDebug>


Simulation::Simulation()
    : frameCount_(0),
      nextUnreadFrame_(0),
      model(new TreeModel(this))
{}

Simulation::~Simulation()
{}

frameNumber_t Simulation::getFrameCount() const
{
    return nextUnreadFrame_ + 1;
}

std::shared_ptr<Frame> Simulation::getFrame(frameNumber_t position)
{
    Frame f = {
        0,
        0,
        std::vector<Atom>(),
        std::map<std::string, float>()
    };
    int count = 0;
    int i = 0;
    for (const auto & l : layerConcatenations_) {
        std::shared_ptr<Frame> f2 = l->getFrame(position);
        f.no = f2->no;
        f.step = f2->step;

        for (const auto& p : f2->functionValues) {
            f.functionValues[p.first + "_layer_" + std::to_string(i)] = p.second;
        }

        for (const auto & atom : f2->atoms) {
            f.atoms.push_back(atom);
            f.atoms.back().id += count;
        }

        for (const auto & connection : f2->connectedRanges) {
            f.connectedRanges.push_back({ connection.first + count, connection.second + count });
        }

        count += f2->atoms.size();
        i++;
    }
//    for (const auto& p : f.connectedRanges) {
//        std::cout << "AGGREGATED CONNECTION: " << p.first << ", " << p.second << std::endl;
//    }

    frameNumber_t nextFrame = getNextTime((position > 0) ? (position - 1) : 0) + 1;
    if (nextUnreadFrame_ < nextFrame) {
        nextUnreadFrame_ = nextFrame;
        emit frameCountChanged(nextUnreadFrame_);
    }

    return std::make_shared<Frame>(f);
}

frameNumber_t Simulation::getNextTime(frameNumber_t time) const
{
    if (layerConcatenations_.empty())
        return 0;

    frameNumber_t maximum = std::numeric_limits<frameNumber_t>::min();
    frameNumber_t minimum = std::numeric_limits<frameNumber_t>::max();
    for (auto & concatenation : layerConcatenations_) {
        frameNumber_t localMinimum = concatenation->getNextTime(time);
        if (localMinimum != time)
            minimum = std::min(minimum, localMinimum);
        maximum = std::max(maximum, localMinimum);
    }

    if (minimum == std::numeric_limits<frameNumber_t>::max())
        return maximum;

    return minimum;
}

frameNumber_t Simulation::getPreviousTime(frameNumber_t time) const
{
    if (layerConcatenations_.empty())
        return 0;

    frameNumber_t maximum = std::numeric_limits<frameNumber_t>::min();
    frameNumber_t minimum = std::numeric_limits<frameNumber_t>::max();
    for (auto & concatenation : layerConcatenations_) {
        frameNumber_t localMaximum = concatenation->getPreviousTime(time);
        if (localMaximum != time)
            maximum = std::max(maximum, localMaximum);
        minimum = std::min(minimum, localMaximum);
    }

    if (maximum == std::numeric_limits<frameNumber_t>::min())
        return minimum;

    return maximum;
}

void Simulation::addSimulationLayerConcatenation(std::shared_ptr<SimulationLayerConcatenation> slc, bool init)
{
    const auto offset = getFrame(0)->atoms.size();
    layerConcatenations_.emplace_back(slc);
    
    connect(layerConcatenations_.back().get(), &SimulationLayerConcatenation::frameCountChanged,
            [this] (int frameCount) {
        if (frameCount_ < frameCount) {
            frameCount_ = frameCount;
            // emit frameCountChanged(frameCount_);
        }
    });

    int layerId = layerConcatenations_.size() - 1;
    slc->setLayerId(layerId);
   
    auto f = slc->getFrame(0);
    model->setupModelData(f->atoms, f->connectedRanges, layerId, offset, init);

}

std::shared_ptr<SimulationLayerConcatenation> Simulation::getSimulationLayerConcatenation(int i)
{
    return layerConcatenations_[i];
}

TreeModel* Simulation::getModel()
{
    return model;
}

#include <QJsonArray>

void Simulation::read(const QJsonArray &json)
{
    for (auto i : json)
    {
        auto simulationLayer = std::make_shared<SimulationLayerConcatenation>();
        simulationLayer->read(i.toArray());

        addSimulationLayerConcatenation(simulationLayer, false);
    }
}

void Simulation::write(QJsonArray &json) const
{
    for (auto i : layerConcatenations_)
    {
        QJsonArray simulationLayer;
        i->write(simulationLayer);

        json.append(simulationLayer);
    }
}
