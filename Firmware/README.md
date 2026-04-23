# Firmware 
This firmware runs on an ESP32-C3 and provides real-time control and monitoring of Tauon Music Player via its HTTP API.

## Working : 
This firmware works in 2 part :
- Fetching Data from the api and showing on the screen
- Taking user input through touch sensor and calling the api for executing commands .

### API :
This firmware uses the tauon player api.    
Some endpoints of the API are :  
- /play  
- /pause   
- /next  
- /back   
- /setvolumerel/<n>  

For more info check : [link](https://github.com/Taiko2k/Tauon/wiki/Remote-Control-API)

### Button Controls :
| Left | Middle | Right |
|------------|------------|------------|
|  Volume -  | Play/Pause |  Volume +  |
| Prev Track |            | Next Track |

### Display :
The UI shows and updates:

- Title and artist : update only when changed
- Playback status : updates on state change
- Progress bar : updates based on position
- Time display : updates every sync cycle
- Volume : updates only when changed

#### Notes :
__Change Wifi Credentials , Pin Mapping ,And IP Address accordingly__ 
