USE [dnworld]
GO
/****** Object:  Table [dbo].[Guilds]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Guilds](
	[GuildID] [int] IDENTITY(1,1) NOT NULL,
	[GuildName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[GuildLevel] [tinyint] NOT NULL,
	[GuildDepotSize] [smallint] NOT NULL,
	[TotalGuildPoint] [int] NOT NULL,
	[RemainGuildPoint] [int] NOT NULL,
	[GuildCoin] [bigint] NOT NULL,
	[NoticeContent] [nvarchar](300) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[CloseFlag] [bit] NOT NULL,
	[CloseDate] [datetime2](0) NULL,
	[TeamColorCode] [tinyint] NULL,
	[GuildWarScheduleID] [smallint] NULL,
	[GuildMarkBG] [smallint] NULL,
	[GuildMarkBorder] [smallint] NULL,
	[GuildMark] [smallint] NULL,
	[TotalGuildEXP] [int] NOT NULL,
	[DailyStagePoint] [int] NOT NULL,
	[StagePointLastModDate] [datetime2](0) NOT NULL,
	[DailyMissionPoint] [int] NOT NULL,
	[MissionPointLastModDate] [datetime2](0) NOT NULL,
	[GuildMaxMemberCount] [int] NOT NULL,
	[GuildHomepageURL] [nvarchar](max) COLLATE Chinese_PRC_CI_AS NULL,
	[GuildTypeCode] [tinyint] NULL,
 CONSTRAINT [PK_Guilds] PRIMARY KEY CLUSTERED 
(
	[GuildID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1] TEXTIMAGE_ON [DataFG1]
GO
/****** Object:  Index [IX_NN_Guilds_GuildTypeCode]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_Guilds_GuildTypeCode] ON [dbo].[Guilds]
(
	[GuildTypeCode] ASC,
	[CloseFlag] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_UNF_Guilds_GuildName]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UNF_Guilds_GuildName] ON [dbo].[Guilds]
(
	[GuildName] ASC
)
WHERE ([CloseFlag]=CONVERT([bit],(0),(0)))
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[Guilds] ADD  CONSTRAINT [DF_Guilds_GuildMaxMemberCount]  DEFAULT ((20)) FOR [GuildMaxMemberCount]
GO
ALTER TABLE [dbo].[Guilds]  WITH CHECK ADD  CONSTRAINT [CK_Guilds_DailyMissionPoint] CHECK  (([DailyMissionPoint]>=(0)))
GO
ALTER TABLE [dbo].[Guilds] CHECK CONSTRAINT [CK_Guilds_DailyMissionPoint]
GO
ALTER TABLE [dbo].[Guilds]  WITH CHECK ADD  CONSTRAINT [CK_Guilds_DailyStagePoint] CHECK  (([DailyStagePoint]>=(0)))
GO
ALTER TABLE [dbo].[Guilds] CHECK CONSTRAINT [CK_Guilds_DailyStagePoint]
GO
ALTER TABLE [dbo].[Guilds]  WITH CHECK ADD  CONSTRAINT [CK_Guilds_GuildCoin] CHECK  (([GuildCoin]>=(0)))
GO
ALTER TABLE [dbo].[Guilds] CHECK CONSTRAINT [CK_Guilds_GuildCoin]
GO
ALTER TABLE [dbo].[Guilds]  WITH CHECK ADD  CONSTRAINT [CK_Guilds_GuildDepotSize] CHECK  (([GuildDepotSize]>=(0)))
GO
ALTER TABLE [dbo].[Guilds] CHECK CONSTRAINT [CK_Guilds_GuildDepotSize]
GO
ALTER TABLE [dbo].[Guilds]  WITH CHECK ADD  CONSTRAINT [CK_Guilds_RemainGuildPoint] CHECK  (([RemainGuildPoint]>=(0)))
GO
ALTER TABLE [dbo].[Guilds] CHECK CONSTRAINT [CK_Guilds_RemainGuildPoint]
GO
ALTER TABLE [dbo].[Guilds]  WITH CHECK ADD  CONSTRAINT [CK_Guilds_TotalGuildEXP] CHECK  (([TotalGuildEXP]>=(0)))
GO
ALTER TABLE [dbo].[Guilds] CHECK CONSTRAINT [CK_Guilds_TotalGuildEXP]
GO
ALTER TABLE [dbo].[Guilds]  WITH CHECK ADD  CONSTRAINT [CK_Guilds_TotalGuildPoint] CHECK  (([TotalGuildPoint]>=(0)))
GO
ALTER TABLE [dbo].[Guilds] CHECK CONSTRAINT [CK_Guilds_TotalGuildPoint]
GO
