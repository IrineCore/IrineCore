USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildHistory]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildHistory](
	[GuildID] [int] NOT NULL,
	[GuildHistoryID] [bigint] IDENTITY(1,1) NOT NULL,
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
	[GuildNameAfter] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NULL,
 CONSTRAINT [PK_GuildHistory] PRIMARY KEY CLUSTERED 
(
	[GuildID] ASC,
	[GuildHistoryID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
