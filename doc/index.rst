
.. title:: Fityk --- curve fitting and peak fitting software

.. meta::
   :description: Fityk. Open-source curve-fitting and data analysis software. Linux, Windows, Mac OS X.
   :keywords: curve fitting, peak fitting, software, Voigt, Doniach-Sunjic

.. role:: smallfont
   :class: smallfont

.. role:: html(raw)
   :format: html

.. image:: fityk076.png
   :alt: [screenshot]
   :align: right
   :scale: 50
   :class: screenshot

Fityk *[fi:tik]* is a program for data processing
and nonlinear **curve fitting**.

It is primarily used...

* by scientists who analyse data from powder diffraction, chromatography,
  photoluminescence and photoelectron spectroscopy,
  infrared and Raman spectroscopy, and other experimental techniques,

..

* to fit peaks -- bell-shaped functions (Gaussian, Lorentzian, Voigt,
  Pearson VII, bifurcated Gaussian, EMG, Doniach-Sunjic, etc.),

... but any functions can be fitted to any 2D (*x*,\ *y*) data.

.. _contents:

:ref:`Features` |
:ref:`Download` |
:ref:`Documentation` |
:ref:`Support <Support>`

.. _Features:

Features
========

* intuitive graphical interface (and also command line interface),
* support for many data file formats, thanks to
  the `xylib library <http://xylib.sourceforge.net/>`_,
* dozens of built-in functions and support for user-defined functions,
* equality constraints,
* fitting systematic errors of the *x* coordinate of points (for example
  instrumental zero error or sample displacement correction
  in powder diffraction),
* manual, graphical placement of peaks and auto-placement using peak detection
  algorithm,
* various optimization methods (standard Marquardt least-squares algorithm,
  Genetic Algorithms, Nelder-Mead simplex),
* handling series of datasets,
* automation with macros (scripts),
* an add-on for powder diffraction data (Pawley refinement)
* modular :wiki:`architecture <Architecture>`,
* open source licence (`GPL <http://creativecommons.org/licenses/GPL/2.0/>`_).

.. _Download:

Download
========

|ico-win| MS Windows: :download-msw:`.exe`

|ico-tux| Fresh Linux RPMs from OBS_ and DEBs from PPA_ or debian-xray_

|ico-osx| Mac OS X: work in progress...

.. _OBS: http://download.opensuse.org/repositories/home://wojdyr/
.. _PPA: https://launchpad.net/~wojdyr/+archive/fityk
.. _debian-xray: http://debian-xray.iit.edu/
.. |ico-win| image:: ico-win.png
.. |ico-tux| image:: ico-tux.png
.. |ico-osx| image:: ico-osx.png


Source code: `GitHub <https://github.com/wojdyr/fityk>`_
:html:`<a class="FlattrButton" style="display:none;" rev="flattr;button:compact;" href="http://fityk.nieto.pl"></a>`

Version 0.9.6 was released on 2011-01-26
(`changelog <https://github.com/wojdyr/fityk/raw/master/NEWS>`_).
Subscribe to `new version notifications <http://fityk-announce.nieto.pl/>`_
(email or feeds).

Subscriptions
-------------

*This is an attempt to make this software self-sustaining and actively
developed in the future.*

After releasing ver. 1.0 new binaries will be available to subscribers only.
Distribution of the source code and the licence will not change.

Subscription cost (includes support via e-mail):

* `1 month subscription <https://www.plimus.com/jsp/buynow.jsp?contractId=2918496>`_: €90 / $115

* `1 year subscription <https://www.plimus.com/jsp/buynow.jsp?contractId=2918202>`_: €210 / $265

* `1 year subscription + 20 hours of coding <https://www.plimus.com/jsp/buynow.jsp?contractId=2918292>`_ €630 / $795.
  :smallfont:`The maintainer of the program will devote up to 20 hours to
  implement feature(s) requested by the customer.
  (Bugs are fixed as soon as possible regardless of who reports them).
  20 hours can be enough to add a simple data file format (2-3 page long spec),
  or to add a new function (like, say, Voigt), or to tweak the GUI.
  If you have a particular feature in mind, please contact us first.`


Alternatively, students and home users may
donate 10% of the normal price to ``wojdyr@gmail.com`` using
`PayPal <https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=E98FRTPDBQ3L6&lc=US&currency_code=USD&item_name=Fityk>`_,
`MoneyBookers <https://www.moneybookers.com/app/payment.pl?pay_to_email=wojdyr@gmail.com&language=EN&detail1_text=The+amount+can+be+changed+at+the+end+of+the+URL&detail1_description=Fityk&currency=USD&amount=26.5>`_
or `Flattr <https://flattr.com/donation/give/to/wojdyr>`_.

You are welcome to make a purchase now, the subscription time will be counted
since the release of ver. 1.0.

.. _Documentation:

Documentation
=============

* `Manual <fityk-manual.html>`_
  (chapters :ref:`intro`, :ref:`getstarted`, :ref:`lang`, :ref:`data`,
  :ref:`model`, :ref:`fit`, :ref:`ref`)
  and the same `in PDF <http://www.unipress.waw.pl/fityk/fityk-manual.pdf>`_,

* `Fityk Wiki <https://github.com/wojdyr/fityk/wiki>`_
  (you are welcome to contribute).

Citing Fityk in academic papers:
M. Wojdyr,
`J. Appl. Cryst. 43, 1126-1128 <http://dx.doi.org/10.1107/S0021889810030499>`_
(2010)
[`reprint <http://www.unipress.waw.pl/fityk/fityk-JAC-10-reprint.pdf>`_]

.. _Support:

Questions?
==========

* Google group `fityk-users <http://groups.google.com/group/fityk-users/>`_
  (you may select "no mail" and use it like forum)

* or wojdyr@gmail.com

Feel free to send questions, comments, bug reports, new feature requests
and success stories.
Asking for a new feature usually results in adding the request to
the `TODO list <https://github.com/wojdyr/fityk/raw/master/TODO>`_
or, if it already is in the list, in assigning higher priority to it.

.. raw:: html

   <script type="text/javascript"> <!--
   if (window != top) top.location.href = location.href;
   $(document).ready(function(){
     $("#features").hide();
     $("#features").prev().after(
      "<p id='expand_features'><a href=''><span class='h1'>Features</span> &nbsp; <span class='smallfont'>[show]</span></a></p>");
     $("#expand_features a").click(function(event){
       $(this).parent().hide();
       $("#features").show('slow');
       event.preventDefault();
     });

     var s = document.createElement('script'), t = document.getElementsByTagName('script')[0];
     s.type = 'text/javascript';
     s.async = true;
     s.src = 'http://api.flattr.com/js/0.6/load.js?mode=auto';
     t.parentNode.insertBefore(s, t);

   });
   //--> </script>


