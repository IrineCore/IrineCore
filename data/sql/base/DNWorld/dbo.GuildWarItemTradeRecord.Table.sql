USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildWarItemTradeRecord]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildWarItemTradeRecord](
	[CharacterID] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemCount] [int] NOT NULL,
 CONSTRAINT [PK_GuildWarItemTradeRecord] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[ItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
