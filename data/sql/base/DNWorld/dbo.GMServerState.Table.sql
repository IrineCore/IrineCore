USE [dnworld]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GMServerState](
	[ProcessIndex] [int] NOT NULL CONSTRAINT [PK_GMServerState] PRIMARY KEY CLUSTERED,
	[ProcessGubun] [smallint] NOT NULL,
	[NetLauncherID] [int] NOT NULL,
	[NetLauncherStatus] [nvarchar](30) NULL,
	[PartitionID] [int] NOT NULL,
	[ProcessType] [nvarchar](30) NULL,
	[AssignedLauncherID] [int] NOT NULL,
	[PStatus] [nvarchar](30) NULL,
	[SStatus] [nvarchar](30) NULL,
	[Version] [nvarchar](30) NULL,
	[Resource] [nvarchar](30) NULL,
	[SID] [int] NOT NULL,
	[UserCount] [int] NOT NULL,
	[RoomCount] [int] NOT NULL
)
GO
