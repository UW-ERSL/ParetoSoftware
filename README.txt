This folder contains the entire suite of Pareto software.

Do not change the hierarchy of directories, or names of directories, since compiler settings rely on this structure.

See the DOCS folder for a high-level software architecture.

The directories are as follows

1. ParetoCore: Contains the code for the Pareto core libraries pareto.lib and paretostatic.lib. The libraries is created via Qt .pro project. The paretostatic.lib is only needed by ParetoCreo, and certain lines must be uncommented in .pro project to create this library.

2. ParetoTopOpt: Uses the pareto.lib, middleware code, and Qt GUI to create the standalone windows topopt application. The application is created via Qt .pro project 

3. ParetoWorks: Uses the pareto.lib, middleware code, and SolidWorks API to create the SOlidWorks plugin topopt dll. The dll is created via VS 2015

4. ParetoCreo: Uses the paretostatic.lib, middleware code, and Creo API to create the Creo plugin topopt dll. The dll is created via VS 2015

