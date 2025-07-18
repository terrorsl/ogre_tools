#include"message_system.h"
#include<Ogre.h>

const size_t MessageQueueSystem::cSizeOfHeader = Ogre::alignToNextMultiple( sizeof( Ogre::uint32 ) * 2, sizeof( size_t ) );
