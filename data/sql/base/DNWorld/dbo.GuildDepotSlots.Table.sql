USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildDepotSlots]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildDepotSlots](
	[GuildID] [int] NOT NULL,
	[ItemSerial] [bigint] NOT NULL,
	[GuildDepotIndex] [smallint] NOT NULL,
 CONSTRAINT [PK_GuildDepotSlots] PRIMARY KEY CLUSTERED 
(
	[GuildID] ASC,
	[ItemSerial] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
