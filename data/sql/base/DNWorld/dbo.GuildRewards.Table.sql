USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildRewards]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildRewards](
	[GuildID] [int] NOT NULL,
	[GuildItemID] [int] NOT NULL,
	[EternityFlag] [bit] NOT NULL,
	[RegisterDate] [datetime2](0) NULL,
	[ItemExpireDate] [datetime2](0) NULL,
 CONSTRAINT [PK_GuildRewards] PRIMARY KEY CLUSTERED 
(
	[GuildID] ASC,
	[GuildItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
