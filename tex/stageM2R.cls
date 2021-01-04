%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%% Class provided by Pascal Giorgi (pascal.giorgi@lirmm.fr (02/2015)                     %%%%%%%
%%%% This class has been inspired by the one used for CSI Master at Université de Bordeaux %%%%%%%
%%%% and written by Emmanuel Fleury <fleury@labri.fr>                                      %%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\NeedsTeXFormat{LaTeX2e}

\ProvidesClass{stageM2R}
[2015/02/01 Classe de mise en page des rapports bibliographiques du M2R Informatique - U. Montpellier]

\newif\if@final \@finalfalse
\DeclareOption{final}{\@finaltrue}
\newif\if@utf \@utffalse
\DeclareOption{utf8}{\@utftrue}
\newif\if@private \@privatefalse
\DeclareOption{private}{\@privatetrue}
\DeclareOption*{}
\ProcessOptions

%% dependance à la classe memoir
\LoadClass[a4paper,11pt,openany]{memoir}

\settrimmedsize{297mm}{210mm}{*}
\setlength{\trimtop}{0pt}
\setlength{\trimedge}{\stockwidth}
\addtolength{\trimedge}{-\paperwidth}
\settypeblocksize{614pt}{418.13pt}{*}
\setulmargins{4cm}{*}{*}
\setlrmargins{*}{*}{1.5}
\setmarginnotes{17pt}{51pt}{\onelineskip}
\setheadfoot{\onelineskip}{2\onelineskip}
\setheaderspaces{*}{2\onelineskip}{*}
\checkandfixthelayout

%% paquets nécessaire
\RequirePackage[french,english]{babel}
\RequirePackage{lmodern}
\if@utf
\RequirePackage[utf8]{inputenc}
\else
\RequirePackage[latin1]{inputenc}
\fi
% \RequirePackageWithOptions{inputenc} % get enconding from class option
\RequirePackage{amstext,amsmath,amssymb,amsfonts}
\RequirePackage{multirow,colortbl}
\RequirePackage{xspace,varioref}
\RequirePackage{hyperref}
\RequirePackage[dvipsnames]{xcolor}
\RequirePackage{graphicx}
\RequirePackage{appendix}
\RequirePackage{makeidx}
\RequirePackage{xparse}

% mot clés et leur traduction
\newcommand{\authors	}[1]{\def\theauthors	{#1}\author{#1}}
\newcommand{\supervisor	}[1]{\def\thesupervisor	{#1}}
\newcommand{\supervisors}[1]{\def\thesupervisors{#1}\supervisor{#1}}
\newcommand{\tutor	}[1]{\def\thetutor	{#1}}
\newcommand{\track	}[1]{\def\thetrack	{#1}}
\newcommand{\version	}[1]{\def\theversion	{#1}}
\newcommand{\abstractfr	}[1]{\def\theabstractfr	{#1}}
\newcommand{\abstracteng}[1]{\def\theabstracteng{#1}}
\newcommand{\location	}[1]{\def\thelocation	{#1}}

\newcommand{\authorname}	{Author}
\newcommand{\authorsname}	{Authors}
\newcommand{\supervisorname}	{Supervisor}
\newcommand{\supervisorsname}	{Supervisors}
\newcommand{\tutorname}		{Tutor}
\newcommand{\trackname}		{Track}
\newcommand{\thepartname}	{Part}
\newcommand{\locationname}	{Trainings location}

\ifdefined\addto{%
	\addto{\captionsfrench}{\renewcommand{\authorname}	{Auteur}}
	\addto{\captionsfrench}{\renewcommand{\authorsname}	{Auteurs}}
	\addto{\captionsfrench}{\renewcommand{\supervisorname}	{Encadrant}}
	\addto{\captionsfrench}{\renewcommand{\supervisorsname}	{Encadrants}}
	\addto{\captionsfrench}{\renewcommand{\tutorname}	{Tuteur}}
	\addto{\captionsfrench}{\renewcommand{\thepartname}	{Partie}}
	\addto{\captionsfrench}{\renewcommand{\trackname}	{Parcours}}
	\addto{\captionsfrench}{\renewcommand{\locationname}	{Lieu de stage}}
	\else{}}
\fi

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%% Page de garde    %%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\pretitle{\flushleft\Huge\textsf}
\posttitle{\\[-.65em]\rule{\linewidth}{1.5mm}\\[-.65em]
	\centering
	\vfill
	\includegraphics{img/logo-um-complet.png}
	\vfill
	\iflanguage{french}{%
		{\Huge\bfseries \if@final Mémoire de fin d'étude\else Présentation du stage recherche\fi}
	}{%
		{\Huge Master Thesis}
	}\\
	\vspace{1.25em}
	\iflanguage{french}{%
		\LARGE
		Master \emph{Sciences et Technologies},\\[.5ex]
		Mention \emph{Informatique},\\[.5ex]
	}{%
		\LARGE
		Master of \emph{Science},\\[.5ex]
		Specialty in \emph{Computer Science},\\[.5ex]
	}
	\ifx\thetrack\undefined%
	\else
	\Large \trackname~\textsc{\thetrack}\\[.5ex]
	\par\hfill
	\fi
} %%% end of posttitle
%% author
\preauthor{\vspace{\fill}\\
	\ifx\theauthors\undefined%
	\flushleft\textbf{\large\authorname}\\
	\else%
	\flushleft\textbf{\large\authorsname}\\
	\fi
	\small
}
%% end of preauthor

\postauthor{\vspace{1em}
	\ifx\thesupervisors\undefined%
	\newline\textbf{\large\supervisorname}\\\thesupervisor\\[1em]%
	\else%
	\newline\textbf{\large\supervisorsname}\\\thesupervisors\\[1em]%
	\fi
	\ifx\thetutor\undefined%
	\else%
	\textbf{\large\tutorname}\\\thetutor%
	\\[-.25em]
	\fi
	\ifx\thelocation\undefined
	\else \ifx \thelocation\empty \else \textbf{\large\locationname}\\\thelocation\\[1em] \fi
	\fi

	\rule{\linewidth}{1mm}\\[-.25em]
}
%% end of postauthor

%% version and date
\predate{\hspace{\fill}
	\ifx\theversion\undefined
	\else%
	\ifx \theversion\empty \else version~\theversion \hfill \fi %
	\fi
	\hfill
	\ifx\thedate\empty \else
	\if@final \textsf{soutenu \if@private à huis clos \else publiquement \fi le }
	\else \fi
}

\postdate{
	%%% Résumé (francais puis anglais)
	\ifx\theabstractfr\undefined \else
	\cleardoublepage
	\begin{abstract}
		\theabstractfr
	\end{abstract}
	\vfill
	\fi
	\ifx\theabstracteng\undefined \else
	\rule{\linewidth}{1mm}
	\selectlanguage{english}
	\begin{abstract}
		\theabstracteng
	\end{abstract}
	\vfill
	\selectlanguage{french}
	\fi
	\thispagestyle{empty}
}
\selectlanguage{french}

%\chapterstyle{veelo}
%\chapterstyle{chappell}
%\chapterstyle{ell}
%\chapterstyle{ger}
%\chapterstyle{pedersen}
%\chapterstyle{verville}
%\chapterstyle{madsen}
\chapterstyle{thatcher}
