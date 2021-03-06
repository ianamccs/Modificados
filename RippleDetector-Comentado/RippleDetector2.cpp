//****************************************************> significa que veio do Bandpass
//**Análise do terminal**
//*O que a função faz*


#include <stdio.h>
#include <math.h>
#include <time.h>
#include <algorithm>
#include "RippleDetector2.h"
#include "RippleDetector2Editor.h"

using namespace std;                                                                                          //Pra não ter q usar std o tempo todo

RippleDetector2::RippleDetector2()
    : GenericProcessor("2Ripple Detector"), activeModule(-1),
//      risingPos(false), risingNeg(false), fallingPos(false), fallingNeg(false),                             //...
//****************************************************
      defaultLowCut(300.0f),                                                                                  //Seta o LowCut inicial
      defaultHighCut(6000.0f)                                                                                 //Seta o HighCut inicial
//****************************************************
{
//****************************************************
    setProcessorType (PROCESSOR_TYPE_FILTER);                                                                 //Deixa o módulo azul claro na borda
//****************************************************

    cout << "------------------------------------------AQUI-1-------------------------------" << endl;        //**Ao colocar o módulo na chain**

}

RippleDetector2::~RippleDetector2()
{
    cout << "------------------------------------------AQUI-2-------------------------------" << endl;        //**Ao TIRAR o módulo da chain**
}                                                                                                             //Destructor

AudioProcessorEditor* RippleDetector2::createEditor()
{
    editor = new RippleDetector2Editor(this, true);

    cout << "------------------------------------------AQUI-3-------------------------------" << endl;        //**Ao colocar o módulo na chain**

    cout << "Creating Editor." << endl;                                                                       //Imprime no terminal
//****************************************************
    RippleDetector2Editor* ed = (RippleDetector2Editor*) getEditor();
    ed->setDefaults (defaultLowCut, defaultHighCut);                                                          //Se tirar isso somem os nº no low/highcut
//****************************************************
    return editor;
}

void RippleDetector2::addModule()
{

    cout << "------------------------------------------AQUI-4-------------------------------" << endl;        //**Ao colocar o módulo na chain**

    DetectorModule m = DetectorModule();
    m.inputChan = -1;
    m.outputChan = -1;
    m.gateChan = -1;
    m.isActive = true;
    m.lastSample = 0.0;
    m.type = NONE;
    m.samplesSinceTrigger = 5000;
    m.wasTriggered = false;
    m.phase = NO_PHASE;
    m.MED = 0.00;
    m.STD = 0.00;
    m.AvgCount = 0;
    m.flag = 0;
    m.tReft = 0.0;
    m.count = 0;
    
    
    modules.add(m);

}


void RippleDetector2::setActiveModule(int i)
{

    cout << "------------------------------------------AQUI-5-------------------------------" << endl;        //**Não aparece printado**

    activeModule = i;

}                                                                                                             


void RippleDetector2::setParameter(int parameterIndex, float newValue)
{

    cout << "------------------------------------------AQUI-6-------------------------------" << endl;      //**Ao colocar o módulo na chain tbm**
                                                                                                              //**Logo antes da falha no JUCE**
                                                                                                              //**Ao mudar o nº de canais input**
                                                                                                              //**Quando muda o low/highcut tbm**
    DetectorModule& module = modules.getReference(activeModule);

    if (parameterIndex == 1) // module type
    {

        int val = (int) newValue;

        switch (val)
        {
            case 0:
                module.type = NONE;
                break;
            case 1:
                module.type = PEAK;
                break;
            case 2:
                module.type = FALLING_ZERO;
                break;
            case 3:
                module.type = TROUGH;
                break;
            case 4:
                module.type = RISING_ZERO;
                break;
            default:
                module.type = NONE;
        }
    }
    else if (parameterIndex == 2)   // inputChan
    {
        module.inputChan = (int) newValue;
    }
    else if (parameterIndex == 3)   // outputChan
    {
        module.outputChan = (int) newValue;
    }
    else if (parameterIndex == 4)   // gateChan
    {
        module.gateChan = (int) newValue;
        if (module.gateChan < 0)
        {
            module.isActive = true; 
        }
        else
        {
            module.isActive = false;
        }
    }

}

void RippleDetector2::updateSettings()
{
    cout << "------------------------------------------AQUI-7-------------------------------" << endl;       //**Ao colocar o módulo na chain**
}                                                                                                             //*Update Settings quando se add módulos à signal chain*

bool RippleDetector2::enable()
{

    cout << "------------------------------------------AQUI-8-------------------------------" << endl;       //**Em Enabling processor**

    return true;
}

void RippleDetector2::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{

    cout << "------------------------------------------AQUI-9-------------------------------" << endl;       //**Em Adding audio callback**
                                                                                                              //**Ao dar play (Varios)**

    if (eventType == TTL)
    {
        const uint8* dataptr = event.getRawData();

        int eventId = *(dataptr+2);
        int eventChannel = *(dataptr+3);

        for (int i = 0; i < modules.size(); i++)
        {
            DetectorModule& module = modules.getReference(i);

            if (module.gateChan == eventChannel)
            {
                if (eventId)
                    module.isActive = true;
                else
                    module.isActive = false;
            }
        }

    }

}

void RippleDetector2::process(AudioSampleBuffer& buffer,
                            MidiBuffer& events) //This is the core of the code, is the script that will run when every buffer comes
{

    cout << "------------------------------------------AQUI-10-------------------------------" << endl;       //**Em Adding audio callback**
                                                                                                              //**Ao dar play (Varios)**

    Time time; //I'm using a library to count time
    checkForEvents(events);
    // loop through the modules
    for (int i = 0; i < modules.size(); i++)
    {
      DetectorModule& module = modules.getReference(i);

        double t;    
        double t3;
        double RefratTime;
            
            
            t = double(time.getHighResolutionTicks()) / double(time.getHighResolutionTicksPerSecond());//Starting to count time for the script here
            double arrSized = round(getNumSamples(module.inputChan)/4);//the following 3 lines are to create an array of specific size for saving the RMS from buffer
            int arrSize = (int) arrSized;
            float RMS[arrSize];
            for (int index = 0; index < arrSize; index++) //here the RMS is calculated
            {
                RMS[index] = sqrt( (
                   pow(buffer.getSample(module.inputChan,(index*4)),2) +
                   pow(buffer.getSample(module.inputChan,(index*4)+1),2) +
                   pow(buffer.getSample(module.inputChan,(index*4)+2),2) +
                   pow(buffer.getSample(module.inputChan,(index*4)+3),2)
                )/4 );
            }
            
            for (int pac = 0; pac < arrSize; pac++)
            {
                if (module.AvgCount < 60000/4) //Using the RMS value in the first 2 s as baseline to build a threshold of detection
                {
                    module.AvgCount++; // all the values that must be saved from one buffer to other has to be save in another function outside the process (this function), when I need to save values to reuse in the next buffer I use the structure module from addModule function
                    float var = RMS[pac];
                    float delta = var - module.MED;
                    module.MED = module.MED + (delta/module.AvgCount); //calculates average for threshold
                    module.STD = module.STD + delta*(var-module.MED); // calculates standard deviation for threhsold
                }
                else
                {
                    break;
                }
            }

            for (int i = 0; i < arrSize; i++)
            {
            
                const float sample = RMS[i];
                double threshold = module.MED + 2.00*sqrt(module.STD/(module.AvgCount*4)); //building the threshold from average + n*standard deviation
                
                if ( sample >=  threshold & RefratTime > 2 ) //counting how many points are above the threshold and if has been 2 s after the last event (refractory period)
                {
                  module.count++;
                }
		else if(sample < threshold & i == 0)//protect from acumulation
		{
		  module.count = 0;
 		}               
                if (module.flag == 1) //if it had a detector activation, starts to recalculate refrat time
                {
                    t3 = ( double(time.getHighResolutionTicks()) / double(time.getHighResolutionTicksPerSecond()) )- module.tReft;//calculating refractory time
                    RefratTime = t3;
                }
                else //if module.flag == 0, then the script is just starting and refractory time is adjusted for a a value that works
                {
                    RefratTime = 3;
                }
       
                if (module.count >= round(0.020*30000/4) & RefratTime > 2 ) //this is the time threshold, buffer RMS amplitude must be higher than threshold for a certain period of time, the second term is the Refractory period for the detection, so it hasn't a burst of activation after reaching both thresholds
                {
			//below from here starts the activation for sending the TTL to the actuator
                        module.flag = 1;
			module.count = 0;
                        module.tReft = double(time.getHighResolutionTicks()) / double(time.getHighResolutionTicksPerSecond());                        
    
                        addEvent(events, TTL, i, 1, module.outputChan);
                        module.samplesSinceTrigger = 0;

                        module.wasTriggered = true;
                        
                }
                module.lastSample = sample;

                if (module.wasTriggered)
                {
                    if (module.samplesSinceTrigger > 1000)
                    {
                        addEvent(events, TTL, i, 0, module.outputChan);
                        module.wasTriggered = false;
                    }
                    else
                    {
                        module.samplesSinceTrigger++;
                    }
                }


        }

    }

}
/*
void RippleDetector2::estimateFrequency()
{
    cout << "------------------------------------------AQUI-11-------------------------------" << endl;
}

//****************************************************
/*void RippleDetector2::updateSettings2()                                                                       //...
{
    int numInputs = getNumInputs();
    int numfilt = filters.size();
   
    cout << "------------------------------------------AQUI-12-------------------------------" << endl;
    
    if (numInputs < 1024 and numInputs != numfilt)
    {
        Array<double> oldlowCuts;
        Array<double> oldhighCuts;
        oldlowCuts = lowCuts;
        oldhighCuts = highCuts;

        filters.clear();
        lowCuts.clear();
        highCuts.clear();

        for (int n = 0; n < getNumInputs(); ++n)
        {
            filters.add (new Dsp::SmoothedFilterDesign
                         <Dsp::Butterworth::Design::BandPass    
                         <2>,                                   
                         1,                                     
                         Dsp::DirectFormII> (1));               


            float newLowCut  = 0.f;
            float newHighCut = 0.f;

            if (oldlowCuts.size() > n)
            {
                newLowCut  = oldlowCuts[n];
                newHighCut = oldhighCuts[n];
            }
            else
            {
                newLowCut  = defaultLowCut;
                newHighCut = defaultHighCut;
            }

            lowCuts.add  (newLowCut);
            highCuts.add (newHighCut);

            setFilterParameters (newLowCut, newHighCut, n);
        }
    }

}

void RippleDetector2::setFilterParameters (double lowCut, double highCut, int chan)
{
    if (channels.size() - 1 < chan)
        return;

    cout << "------------------------------------------AQUI-13-------------------------------" << endl;

    Dsp::Params params;
    params[0] = channels[chan]->sampleRate; // sample rate
    params[1] = 2;                          // order
    params[2] = (highCut + lowCut) / 2;     // center frequency
    params[3] = highCut - lowCut;           // bandwidth
    
    if (filters.size() > chan)
        filters[chan]->setParams (params);
}

void RippleDetector2::setParameter2 (int parameterIndex, float newValue)
{

    cout << "------------------------------------------AQUI-14-------------------------------" << endl;

    if (parameterIndex < 2) // change filter settings
    {
        if (newValue <= 0.01 || newValue >= 10000.0f)
            return;

        if (parameterIndex == 0)
        {
            lowCuts.set (currentChannel,newValue);
        }
        else if (parameterIndex == 1)
        {
            highCuts.set (currentChannel,newValue);
        }

        setFilterParameters (lowCuts[currentChannel],
                             highCuts[currentChannel],
                             currentChannel);

        editor->updateParameterButtons (parameterIndex);
    }
}

void RippleDetector2::process2 (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{

    cout << "------------------------------------------AQUI-15-------------------------------" << endl;

    for (int n = 0; n < getNumOutputs(); ++n)
    {
            float* ptr = buffer.getWritePointer (n);
            filters[n]->process (getNumSamples (n), &ptr);
    }
}

void RippleDetector2::saveCustomChannelParametersToXml (XmlElement* channelInfo, int channelNumber, bool isEventChannel)
{

    cout << "------------------------------------------AQUI-16-------------------------------" << endl;       //**Ao fechar o OE**

    if (! isEventChannel
        && channelNumber > -1
        && channelNumber < highCuts.size())
    {

        XmlElement* channelParams = channelInfo->createNewChildElement ("PARAMETERS");
        channelParams->setAttribute ("highcut",         highCuts[channelNumber]);
        channelParams->setAttribute ("lowcut",          lowCuts[channelNumber]);
    }
}


void RippleDetector2::loadCustomChannelParametersFromXml (XmlElement* channelInfo, bool isEventChannel)
{

    cout << "------------------------------------------AQUI-17-------------------------------" << endl;       //**Ao abrir Signal Chain salva**

    int channelNum = channelInfo->getIntAttribute ("number");

    if (! isEventChannel)
    {
        forEachXmlChildElement (*channelInfo, subNode)
        {
            if (subNode->hasTagName ("PARAMETERS"))
            {
                highCuts.set (channelNum, subNode->getDoubleAttribute ("highcut", defaultHighCut));
                lowCuts.set  (channelNum, subNode->getDoubleAttribute ("lowcut",  defaultLowCut));

                setFilterParameters (lowCuts[channelNum], highCuts[channelNum], channelNum);
            }
        }
    }
}*/
//****************************************************
