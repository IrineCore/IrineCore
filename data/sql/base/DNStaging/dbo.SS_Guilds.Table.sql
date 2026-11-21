USE [dnstaging]
GO
/****** Object:  Table [dbo].[SS_Guilds]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SS_Guilds](
	[WorldID] [int] NOT NULL,
	[GuildID] [int] NOT NULL,
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
	[TotalGuildEXP] [int] NULL,
	[GuildMaxMemberCount] [int] NULL
) ON [PRIMARY]
GO
