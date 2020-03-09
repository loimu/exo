/* ========================================================================
*	Copyright (C) 2013-2020 Blaze <blaze@vivaldi.net>
*
*	This file is part of eXo.
*
*	eXo is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	eXo is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with eXo.  If not, see <http://www.gnu.org/licenses/>.
* ======================================================================== */

#ifndef MOC_PROTOCOL_H
#define MOC_PROTOCOL_H


/*
 * Events
*/
#define EV_STATE		0x01 // server changed the state
#define EV_CTIME		0x02 // current time of the song has changed
#define EV_SRV_ERROR	0x04 // an error occurred
#define EV_BUSY			0x05 // another client is connected to the server
#define EV_DATA			0x06 // data in response to a request will arrive
#define EV_BITRATE		0x07 // bitrate has changed
#define EV_RATE			0x08 // rate has changed
#define EV_CHANNELS		0x09 // number of channels has changed
#define EV_EXIT			0x0a // server is about to exit
#define EV_PONG			0x0b // response for CMD_PING
#define EV_OPTIONS		0x0c // options has changed
#define EV_SEND_PLIST	0x0d // request for sending the playlist
#define EV_TAGS			0x0e // tags for the current file have changed
#define EV_STATUS_MSG	0x0f // followed by a status message
#define EV_MIXER_CHANGE	0x10 // mixer channel was changed
#define EV_FILE_TAGS	0x11 // tags in a response for tags request
#define EV_AVG_BITRATE	0x12 // average bitrate has changed (new song)
#define EV_AUDIO_START	0x13 // playing of audio has started
#define EV_AUDIO_STOP	0x14 // playing of audio has stopped
/* Events sent by client to keep playlist in sync */
#define EV_PLIST_ADD	0x50 // add an item, followed by the file name
#define EV_PLIST_DEL	0x51 // delete an item, followed by the file name
#define EV_PLIST_MOVE	0x52 // move an item, followed by 2 file names
#define EV_PLIST_CLEAR	0x53 // clear the playlist
/* Events sent by server to keep playlist in sync */
#define EV_QUEUE_ADD	0x54
#define EV_QUEUE_DEL	0x55
#define EV_QUEUE_MOVE	0x56
#define EV_QUEUE_CLEAR	0x57

/*
 * Server state
*/
#define STATE_PLAY	0x01
#define STATE_STOP	0x02
#define STATE_PAUSE	0x03

/*
 * Commands
*/
#define CMD_PLAY			0x00 // play the first element on the list
#define CMD_LIST_CLEAR		0x01 // clear the list
#define CMD_LIST_ADD		0x02 // add an item to the list
#define CMD_STOP			0x04 // stop playing
#define CMD_PAUSE			0x05 // pause
#define CMD_UNPAUSE			0x06 // unpause
#define CMD_SET_OPTION		0x07 // set an option
#define CMD_GET_OPTION		0x08 // get an option
#define CMD_GET_CTIME		0x0d // get the current song time
#define CMD_GET_SNAME		0x0f // get the stream file name
#define CMD_NEXT			0x10 // start playing next song if available
#define CMD_QUIT			0x11 // shutdown the server
#define CMD_SEEK			0x12 // seek in the current stream
#define CMD_GET_STATE		0x13 // get the state
#define CMD_DISCONNECT		0x15 // disconnect from the server
#define CMD_GET_BITRATE		0x16 // get the bitrate
#define CMD_GET_RATE		0x17 // get the rate
#define CMD_GET_CHANNELS	0x18 // get the number of channels
#define CMD_PING			0x19 // request for EV_PONG
#define CMD_GET_MIXER		0x1a // get the volume level
#define CMD_SET_MIXER		0x1b // set the volume level
#define CMD_DELETE			0x1c // delete an item from the playlist
#define CMD_SEND_PLIST_EVENTS 0x1d // request for playlist events
#define CMD_PREV			0x20 // start playing previous song if available
#define CMD_SEND_PLIST		0x21 // send the playlist to the requesting client
#define CMD_GET_PLIST		0x22 // get the playlist from one of the clients
#define CMD_CAN_SEND_PLIST	0x23 // mark the client as able to send playlist
#define CMD_CLI_PLIST_ADD	0x24 // add an item to the client playlist
#define CMD_CLI_PLIST_DEL	0x25 // delete an item from the client playlist
#define CMD_CLI_PLIST_CLEAR	0x26 // clear the client playlist
#define CMD_GET_SERIAL		0x27 // get an unique serial number
#define CMD_PLIST_SET_SERIAL 0x28 // assign a serial number to the server playlist
#define CMD_LOCK			0x29 // acquire a lock
#define CMD_UNLOCK	  		0x2a // release the lock
#define CMD_PLIST_GET_SERIAL 0x2b // get a serial number of the server playlist
#define CMD_GET_TAGS		0x2c // get tags for the currently played file
#define CMD_TOGGLE_MIXER_CHANNEL 0x2d // toggle the mixer channel
#define CMD_GET_MIXER_CHANNEL_NAME 0x2e // get the mixer channel name
#define CMD_GET_FILE_TAGS	0x2f // get tags for the specified file
#define CMD_ABORT_TAGS_REQUESTS	0x30 // abort the previous CMD_* request
#define CMD_CLI_PLIST_MOVE	0x31 // move an item
#define CMD_LIST_MOVE		0x32 // move an item
#define CMD_GET_AVG_BITRATE	0x33 // get the average bitrate

#define CMD_TOGGLE_SOFTMIXER	0x34 // toggle use of softmixer
#define CMD_TOGGLE_EQUALIZER	0x35 // toggle use of equalizer
#define CMD_EQUALIZER_REFRESH	0x36 // refresh EQ-presets
#define CMD_EQUALIZER_PREV	  	0x37 // select previous eq-preset
#define CMD_EQUALIZER_NEXT	  	0x38 // select next eq-preset

#define CMD_TOGGLE_MAKE_MONO 0x39 // toggle mono mixing
#define CMD_JUMP_TO	 		0x3a // jumps to a some position in the current stream
#define CMD_QUEUE_ADD		0x3b // add an item to the queue
#define CMD_QUEUE_DEL		0x3c // delete an item from the queue
#define CMD_QUEUE_MOVE		0x3d // move an item in the queue
#define CMD_QUEUE_CLEAR		0x3e // clear the queue
#define CMD_GET_QUEUE		0x3f // request the queue from the server


#endif // MOC_PROTOCOL_H
