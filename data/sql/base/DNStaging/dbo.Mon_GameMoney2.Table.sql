USE [dnstaging]
GO
/****** Object:  Table [dbo].[Mon_GameMoney2]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Mon_GameMoney2](
	[BasisDate] [date] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[GameMoneyAmount] [bigint] NOT NULL,
 CONSTRAINT [PK_Mon_GameMoney2] PRIMARY KEY CLUSTERED 
(
	[BasisDate] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
