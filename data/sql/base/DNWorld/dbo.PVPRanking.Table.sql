USE [dnworld]
GO
/****** Object:  Table [dbo].[PVPRanking]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PVPRanking](
	[CharacterID] [bigint] NOT NULL,
	[CharacterName] [nvarchar](30) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[CharacterClassCode] [tinyint] NOT NULL,
	[CharacterSubClassCode] [tinyint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[GuildID] [int] NULL,
	[GuildName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NULL,
	[PVPExp] [int] NOT NULL,
	[Kill] [int] NOT NULL,
	[Death] [int] NOT NULL,
	[PVPClassRank] [int] NOT NULL,
	[PVPClassRatio] [float] NOT NULL,
	[TotalRank] [bigint] NOT NULL,
	[CharacterClassRank] [bigint] NOT NULL,
	[CharacterSubClassRank] [bigint] NOT NULL,
	[ChangedTotalRank] [bigint] NULL,
	[ChangedCharacterClassRank] [bigint] NULL,
	[ChangedCharacterSubClassRank] [bigint] NULL,
	[CharacterLevel] [tinyint] NULL,
	[PVPLevel] [tinyint] NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_PVPRanking_TotalRank]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_PVPRanking_TotalRank] ON [dbo].[PVPRanking]
(
	[TotalRank] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [PK_PVPRanking]    Script Date: 11/14/2026 8:54:37 AM ******/
ALTER TABLE [dbo].[PVPRanking] ADD  CONSTRAINT [PK_PVPRanking] PRIMARY KEY NONCLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_PVPRanking_CharacterClassRank]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PVPRanking_CharacterClassRank] ON [dbo].[PVPRanking]
(
	[CharacterClassRank] ASC,
	[CharacterClassCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_NN_PVPRanking_CharacterName]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PVPRanking_CharacterName] ON [dbo].[PVPRanking]
(
	[CharacterName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_PVPRanking_CharacterSubClassRank]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PVPRanking_CharacterSubClassRank] ON [dbo].[PVPRanking]
(
	[CharacterSubClassRank] ASC,
	[CharacterSubClassCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_PVPRanking_GuildID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PVPRanking_GuildID] ON [dbo].[PVPRanking]
(
	[GuildID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_NN_PVPRanking_GuildName]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PVPRanking_GuildName] ON [dbo].[PVPRanking]
(
	[GuildName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
