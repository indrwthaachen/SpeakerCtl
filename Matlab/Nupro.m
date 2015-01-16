classdef Nupro
    %NUPRO Matlab Class for communication with TrayTask
    
    properties
        ioDev
    end

    
    methods
      function obj = Nupro()
        
        obj.ioDev = tcpip('localhost', 9999); 
		set(obj.ioDev, 'InputBufferSize', 20000);

        fopen(obj.ioDev); 
	    %set(obj.ioDev, 'Timeout', 80);
      end

      
     function out = readResponse(obj)

         out = '';
         line = fgets(obj.ioDev);
         out = strcat(out, [ sprintf('\n'),  line]);
    
         set(obj.ioDev, 'Timeout', 10);
      end  

      function sendCMD(obj, cmd)
		
	while(obj.ioDev.BytesAvailable > 0)
		fread(obj.ioDev, obj.ioDev.BytesAvailable);
        end
        flushinput(obj.ioDev);
        fprintf(obj.ioDev,cmd);
	pause(0.6);
      end  


      function out = disableMissCheck(obj)
         obj.sendCMD( 'all disableMissCheck');
         out = obj.readResponse();          
      end  
      
      function out = setVolumeAll(obj, dbVolume)
         obj.disableMissCheck();
         obj.sendCMD( ['all setVolume ', int2str(dbVolume)]);
         disp(['all setVolume ', int2str(dbVolume)]);
         set(obj.ioDev, 'Timeout', 80);
         responses = obj.readResponse();
		 responses = strsplit(responses,'|');
		 out = '';
		 for response = responses
			if(isempty(strfind(response{1}, 'Ok')))
				out = [out, 'failed '];
			else
				out = [out, 'ok '];
			end
		 end
      end

      function out = setVolume(obj, speakerName, dbVolume)
        
         obj.disableMissCheck();
         obj.sendCMD( [speakerName, ' setVolume ', int2str(dbVolume)]);
         disp([speakerName, '  setVolume ', int2str(dbVolume)]);
         set(obj.ioDev, 'Timeout', 80);
         out = obj.readResponse();
		 if(isempty(strfind(out, 'Ok')))
			out = 'failed';
		 else
			out = 'ok';
		end
	  end

      function out = getVolume(obj, speakerName)
         obj.sendCMD( [speakerName, ' getVolume']);
         out = obj.readResponse();
      end

	  function out = getSignature(obj, speakerName)
         obj.sendCMD( [speakerName, ' Signature']);
         out = obj.readResponse();
      end
	  
      function out = getVolumeAll(obj)
         obj.sendCMD('all getVolume');
         out = obj.readResponse();
		 out = strrep(out, ' | ', '');
      end

      
      function out = getSpeakers(obj)
         obj.sendCMD('getSpeakers');
         out = obj.readResponse();		 
      end

      function out = resetVolumeAll(obj)
         obj.sendCMD('all resetVolume');
         set(obj.ioDev, 'Timeout', 80);
         resp = obj.readResponse();
      end

      function out = resetVolume(obj, speakerName)
         obj.sendCMD( [speakerName, ' resetVolume ']);
         set(obj.ioDev, 'Timeout', 80);
         resp = obj.readResponse();
      end

      function out = setInputAll(obj, input)
          
          switch(input)
              case 'usb'
                  obj.sendCMD( 'all USB');
              case 'aux'
                  obj.sendCMD( 'all AUX');
              case 'spdif'
                  obj.sendCMD( 'all SPDIF');
              otherwise
                  error('input name is invalid (valid: usb/aux/spdif)');                             
          end

         out = obj.readResponse();
		 out = strrep(out, ' | ', '');
      end

      function out = setInput(obj, speakerName, input)
          
          switch(input)
              case 'usb'
                  obj.sendCMD( [speakerName, ' USB']);
              case 'aux'
                  obj.sendCMD( [speakerName, ' AUX']);
              case 'spdif'
                  obj.sendCMD( [speakerName, ' SPDIF']);
              otherwise
                  error('input name is invalid (valid: usb/aux/spdif)');                  
          end

         out = obj.readResponse();
      end

      function out = limit(obj, speakerName)
         obj.sendCMD( [speakerName, ' ?']);
         out = obj.readResponse();         
      end      

      function out = limitAll(obj)
         obj.sendCMD( 'all ?');
         out = obj.readResponse();
		 out = strrep(out, ' | ', '');    
      end       
     
      function delete(obj)
         fclose(obj.ioDev);
      end 
    end
    
end

