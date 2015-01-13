#include "World/EnergyPoint.h"

#include "RoboroboMain/roborobo.h"
#include "Utilities/Misc.h"
#include "Utilities/SDL_gfxPrimitives.h"

#include "World/World.h"

int EnergyPoint::_nextId = 0;

EnergyPoint::EnergyPoint()
{
	_position = gEnergyDistribution->draw();
	_xCenterPixel = _position.x;
	_yCenterPixel = _position.y;

	_id = EnergyPoint::_nextId; // set unique id.
	EnergyPoint::_nextId++; // update id reference.
	
	_fixedLocation = false;
	_active = true;
	
	gProperties.checkAndGetPropertyValue("initLock", &_initLock, true);
	_key = _initLock;

	_radius = gEnergyPointRadius;

	gProperties.checkAndGetPropertyValue("iterationMax", &_iterationMax, true);

	_respawnLag = gEnergyPointRespawnLagMaxValue ;
	_internLagCounter = 0;
	
	_energyPointValue = gEnergyPointValue;
	_energyPointValueIsLocal = false; // use gEnergyPointValue

	_respawnMethodIsLocal = false; // use gEnergyPointRespawnLagMaxValue
	_energyPointRespawnLagMaxValue = gEnergyPointRespawnLagMaxValue; // default, not used if _respawnMethodIsLocal

	gProperties.checkAndGetPropertyValue("VisibleEnergyPoint", &_visible, true);
	if ( _visible)
	{
		display();
	}
}

EnergyPoint::EnergyPoint(int id)
{
	_id = id;
	double x = 0.0, y = 0.0;

	std::string s = "";
	s += "energy[";
	std::stringstream out;
	out << _id;
	s += out.str();
	s += "].x";
	if ( gProperties.hasProperty( s ) )
	{
		convertFromString<double>(x, gProperties.getProperty( s ), std::dec);
	}
	else
	{
		if ( gVerbose )
		{
			std::cout << "[warning] Energy point #" << _id << " - X coordinate not found, pick random value." << std::endl;
		}
		x = (rand() % (gAreaWidth-20)) + 10;
	}

	s = "energy[";
	s += out.str();
	s += "].y";
	if ( gProperties.hasProperty( s ) )
	{
		convertFromString<double>(y, gProperties.getProperty( s ), std::dec);
	}
	else
	{
		if ( gVerbose )
		{
			std::cout << "[warning] Energy point #" << _id << " - Y coordinate not found, pick random value." << std::endl;
		}
		y = (rand() % (gAreaHeight-20)) + 10;
	}

	_position = Point2d(x,y);
	_xCenterPixel = x;
	_yCenterPixel = y;

	_fixedLocation = true;
	_active = true;

	gProperties.checkAndGetPropertyValue("initLock", &_initLock, true);
	_key = _initLock;

	gProperties.checkAndGetPropertyValue("gEnergyPointRadius", &_radius, true);

	gProperties.checkAndGetPropertyValue("iterationMax", &_iterationMax, true);

	_respawnLag = gEnergyPointRespawnLagMaxValue ;
	_internLagCounter = 0;
	
	_respawnMethodIsLocal = false; // use gEnergyPointRespawnLagMaxValue
	_energyPointRespawnLagMaxValue = gEnergyPointRespawnLagMaxValue; // default, not used if _respawnMethodIsLocal
	
	_energyPointValue = gEnergyPointValue;
	_energyPointValueIsLocal = false; // use gEnergyPointValue

	gProperties.checkAndGetPropertyValue("VisibleEnergyPoint", &_visible, false);
    
	if ( _visible)
	{
		display();
	}

}

EnergyPoint::~EnergyPoint()
{
	//nothing to do
}

Point2d EnergyPoint::getPosition()
{
	return _position;
}

void EnergyPoint::display()
{
	Uint32 color = gEnergyPointColor;
	for (Sint16 xColor = _xCenterPixel - Sint16(_radius) ; xColor < _xCenterPixel + Sint16(_radius) ; xColor++)
	{
		for (Sint16 yColor = _yCenterPixel - Sint16(_radius) ; yColor < _yCenterPixel + Sint16 (_radius); yColor ++)
		{
			if ((sqrt ( pow (xColor-_xCenterPixel,2) + pow (yColor - _yCenterPixel,2))) < _radius)
			{
				pixelColor(gForegroundImage, xColor, yColor, color);
				pixelColor(gEnergyImage, xColor, yColor, color);
			}
		}
	}
}

void EnergyPoint::hide()
{
	Uint32 color = 0xffffffff;
	for (Sint16 xColor = _xCenterPixel - Sint16(_radius) ; xColor < _xCenterPixel + Sint16(_radius) ; xColor++)
	{
		for (Sint16 yColor = _yCenterPixel - Sint16(_radius) ; yColor < _yCenterPixel + Sint16 (_radius); yColor ++)
		{
			if ((sqrt ( pow (xColor-_xCenterPixel,2) + pow (yColor - _yCenterPixel,2))) < _radius)
			{
				pixelColor(gForegroundImage, xColor, yColor, color);
				pixelColor(gEnergyImage, xColor, yColor, color);
			}
		}
	}

    // re-display energypoints that we overlapped
    for (size_t i=0; i<gEnergyPoints.size(); i++) {
        if(gEnergyPoints[i]._active && gEnergyPoints[i]._id != this->_id&& getEuclidianDistance(gEnergyPoints[i]._xCenterPixel, gEnergyPoints[i]._yCenterPixel, this->_xCenterPixel, this->_yCenterPixel) < (gEnergyPoints[i]._radius + this->_radius)){
            gEnergyPoints[i].display();
        }
    }
}

int EnergyPoint::getRespawnLag()
{
	return gEnergyPointRespawnLagMaxValue;
}

void EnergyPoint::setActiveStatus( bool __value )
{
	if ( __value == true )
	{
		_active = true;
		if ( _visible )
		{
			display();
		}
	}
	else
	{	
		_active = false;
	
		if ( _visible )
		{
			hide();
		}
		if(gVerbose)
			gLogFile << gWorld->getIterations() << " :: EnergyPoint nb." << _id << " #internLag: " << _respawnLag << std::endl; // This is not relevant - internLag does not exist anymore, this should be either _respawnLag or gEnergyPointRespawnLagMaxValue (ie. time to respawn)
	}	
}

bool EnergyPoint::getActiveStatus()
{
	return _active;
}

float EnergyPoint::getKey()
{
	return _key;
}

void EnergyPoint::setKey(float key)
{
	_key = key;
}

void EnergyPoint::setRespawnLag( int __lag )
{
	_respawnLag = __lag;
}

int EnergyPoint::getRespawnLagMaxValue( )
{
	return _energyPointRespawnLagMaxValue;
}

void EnergyPoint::setRespawnLagMaxValue( int __lagMaxValue )
{
	setRespawnLagMethodIsLocal(true); // automatically set-up local value (if needed)
	_energyPointRespawnLagMaxValue = __lagMaxValue;
}

void EnergyPoint::setRespawnLagMethodIsLocal( bool __flag )
{
	_respawnMethodIsLocal = __flag;
}

bool EnergyPoint::getRespawnLagMethodIsLocal()
{
	return 	_respawnMethodIsLocal;
}

bool EnergyPoint::getEnergyPointValueIsLocal()
{
	return _energyPointValueIsLocal;
}

void EnergyPoint::setEnergyPointValueIsLocal( bool __flag )
{
	_energyPointValueIsLocal = __flag;
}

int EnergyPoint::getEnergyPointValue()
{
	return _energyPointValue;
}

void EnergyPoint::setEnergyPointValue( int __value )
{
	setEnergyPointValueIsLocal(true);
	_energyPointValue = __value;
}

void EnergyPoint::step()
{
	if ( !_active ) // case: harvested. respawn delay?
	{
		_internLagCounter++;

		if (_respawnLag != -1 &&  _internLagCounter >= _respawnLag ) // Note: in current implementation, _respawnLag is equal to gEnergyPointRespawnLagMaxValue.
		{
			if ( _respawnMethodIsLocal == true )
				_respawnLag = _energyPointRespawnLagMaxValue; // each energy point gets its own respawn lag
			else
				_respawnLag = gEnergyPointRespawnLagMaxValue; // all energy points share the respawn lag value

			_internLagCounter = 0;
			_active = true;

			if ( ! _fixedLocation )
			    setPosition(gEnergyDistribution->draw());

			if ( _visible )
			{
				display();
			}
		}
	}
	else
	{
		if ( _visible && gDisplayMode == 0 )
		{
			// NOTE: calling the display method, re-display the food image (but have
			// no impact on simulation whatsoever as food harvesting is based on 
			// computed euclidian distance btw agents and energy points). This is used
			// to guarantee a "nice and clean" display as overlapping energy points 
			// may result in cropped remaining points whenever a point is taken (ie.
			// the actual implementation implies deleting the energy point region
			// that was harvested, without any regards to overlaps).
			// IT IS IMPORTANT TO NOTE THAT SYSTEMATIC CALLS TO DISPLAY MAY 
			// SIGNIFICANTLY SLOW DOWN THE SIMULATION! As a consequence, it should
			// be called with care, and should NOT be called if not be called when
			// the simulation is running fast (e.g. batch mode).
			// Current implementation: 
			//	calls only if display mode is normal speed
			//	and if property "EnergyPoints_alwaysRender" is true
			
			if ( gEnergyPoints_alwaysRender ) // NOTE: the hide() method but be called externally (otw: displayed points may not exist anymore)
				display();
		}
	}
}

void EnergyPoint::setPosition (Point2d __position)
{
	_position = __position;
	_xCenterPixel = __position.x;
	_yCenterPixel = __position.y;
}

void EnergyPoint::setFixedLocationStatus( bool __value )
{
	_fixedLocation = __value;
}

bool EnergyPoint::getFixedLocationStatus()
{
	return _fixedLocation;
}

