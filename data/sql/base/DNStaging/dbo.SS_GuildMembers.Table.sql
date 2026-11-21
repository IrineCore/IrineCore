USE [dnstaging]
GO
/****** Object:  Table [dbo].[SS_GuildMembers]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SS_GuildMembers](
	[WorldID] [int] NOT NULL,
	[GuildID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[GuildMemberRoleCode] [tinyint] NOT NULL,
	[SelfIntroduction] [nvarchar](300) COLLATE Chinese_PRC_CI_AS NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[TotalGuildPoint] [int] NOT NULL,
	[UsedGuildPoint] [int] NOT NULL
) ON [PRIMARY]
GO
