USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtPvpRank]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtPvpRank](
	[id] [int] NULL,
	[PvPRank] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PvPRankEXP] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PvPRankIMG] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PvPRankUIString] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[RankType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PVPRankValue] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[UIStringPvPRankUIString] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtPvpRank_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtPvpRank_id] ON [dbo].[ExtPvpRank]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
