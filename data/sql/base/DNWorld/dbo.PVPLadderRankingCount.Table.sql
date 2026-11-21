USE [dnworld]
GO
/****** Object:  Table [dbo].[PVPLadderRankingCount]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PVPLadderRankingCount](
	[GuildName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NULL,
	[PVPLadderCode] [tinyint] NULL,
	[CharacterClassCode] [tinyint] NULL,
	[CharacterSubClassCode] [tinyint] NULL,
	[Total] [tinyint] NULL,
	[TotalCount] [bigint] NOT NULL
) ON [DataFG1]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_NC_PVPLadderRankingCount_GuildName]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_PVPLadderRankingCount_GuildName] ON [dbo].[PVPLadderRankingCount]
(
	[GuildName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
