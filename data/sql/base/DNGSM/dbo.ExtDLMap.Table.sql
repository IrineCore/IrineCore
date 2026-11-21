USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtDLMap]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtDLMap](
	[id] [int] NULL,
	[_GateEnterImage] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_StageCount] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_FloorCount] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_StartRound] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_FloorSet] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ChallengeType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_GameModeName] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtDLMap_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtDLMap_id] ON [dbo].[ExtDLMap]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
