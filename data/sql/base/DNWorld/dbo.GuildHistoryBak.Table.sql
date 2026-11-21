USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildHistoryBak]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildHistoryBak](
	[GuildID] [int] NOT NULL,
	[GuildHistoryID] [bigint] NOT NULL,
	[GuildHistoryCode] [tinyint] NOT NULL,
	[CharacterID] [bigint] NULL,
	[CharacterLevel] [tinyint] NULL,
	[ItemID] [int] NULL,
	[ItemLevel] [tinyint] NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[GuildLevel] [tinyint] NULL,
	[UsedGuildPoint] [int] NULL,
	[GuildMemberRoleCode] [tinyint] NULL,
	[GuildDepotSize] [smallint] NULL,
	[GuildNameBefore] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NULL,
	[GuildNameAfter] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NULL
) ON [DataFG1]
GO
