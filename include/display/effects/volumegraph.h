#ifndef volumegraph_h
#define volumegraph_h

/* Project Scope */
#include "display/canvas.h"
#include "display/effects/effect.h"

class VolumeGraph : public DisplayEffect {
public:
    VolumeGraph(const canvas::Canvas& size);
    canvas::Canvas run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final;

private:
    canvas::Canvas _c;
    bool _finished = false;
};

#endif // volumegraph_h