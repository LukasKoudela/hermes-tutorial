Multimesh *hp*-FEM
------------------

In multiphysics PDE problems it often happens that one
physical field has a singularity or a boundary layer 
where other fields are smooth. If one approximates all of them on the 
same mesh, then some of them will be refined where this is absolutely not needed.
This can be extremely wasteful, as we will see in the tutorial example "02-system-adapt". 
But first let us introduce the original multimesh discretization method that we developed 
to circumvent this problem.

Multimesh assembling
~~~~~~~~~~~~~~~~~~~~

Hermes makes it possible to approximate each physical field (or solution
component) on an individual mesh. These meshes are not completely independent
of each other -- they have a common coarse mesh that we call *master mesh*.
The master mesh is there for algorithmic purposes only, it may not 
even be used for discretization purposes: Every mesh in the system 
is obtained from it via an arbitrary sequence of elementary refinements.
This is illustrated in the following figure, where (A) is the master mesh,
(B) - (D) three different meshes (say, for a coupled problem with three
equations), and (E) is the virtual *union mesh* that is used for assembling.

.. figure:: intro/multimesh.png
   :align: center
   :scale: 50% 
   :figclass: align-center
   :alt: Multimesh

The union mesh is not constructed physically in the computer memory -- 
merely it serves as a hint to correctly transform integration points
while integrating over sub-elements of the elements of the existing meshes. 
The following figure shows the integration over an element $Q_k$ of the 
virtual union mesh, and what are the appropriate subelements of the 
existing elements where this integration is performed:

.. figure:: intro/multimesh2.png
   :align: center
   :scale: 50% 
   :figclass: align-center
   :alt: Multimesh

As a result, the multimesh discretization of the PDE system is *monolithic*
in the sense that *no physics is lost* -- all integrals in the 
discrete weak formulations are evaluated exactly up to the error in the 
numerical quadrature. In particular, we do not perform operator splitting 
or commit errors while transferring solution data between different meshes.
The multimesh assembling in Hermes works with all meshes at the same time, 
there is no such thing as interpolating or projecting functions between 
different meshes. More details about this method can be found in the 
corresponding research articles.

Simultaneous adaptivity on multiple meshes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As explained above, each physical field (solution component0 has its own mesh 
and these meshes can be very different. The adaptivity algorithm puts all elements 
of all these meshes one single array, and calculates an error function as a difference
between the coarse and reference mesh approximations. Elements are then sorted according 
to their norm of the error function, and then the ones with the largest errors are refined
as in the standard hp-FEM. Note that if some physical field is already resolved well, 
this algorithm leaves its mesh alone and will only perform refinements in the other 
meshes where they are needed. 

