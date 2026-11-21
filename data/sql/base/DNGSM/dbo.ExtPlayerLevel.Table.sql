USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtPlayerLevel]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtPlayerLevel](
	[id] [int] NULL,
	[_Strength] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Agility] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Intelligence] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Stamina] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Experience] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SuperAmmor] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SkillPoint] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Fatigue] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_DeadDurabilityRatio] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_KillScore] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Assistscore] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Assistdecision] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Aggroper] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_WeekFatigue] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SKillUsageRatio] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_IllegalAmount] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_BanAmount] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_AggroperPvE] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_VIPFatigue] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_VIPExp] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_LevelDExp] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtPlayerLevel_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtPlayerLevel_id] ON [dbo].[ExtPlayerLevel]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
