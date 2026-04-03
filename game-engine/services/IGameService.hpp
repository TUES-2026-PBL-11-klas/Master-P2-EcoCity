#ifndef IGAME_SERVICE_H
#define IGAME_SERVICE_H

class IGameService {
public:
    virtual bool tick() = 0;
    virtual void readPlayerInput() = 0;
    virtual ~IGameService() = default;
};

#endif
