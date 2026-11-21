USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_DailyGameMoney]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_DailyGameMoney](
	[BasisDate] [date] NOT NULL,
	[WorldID] [int] NOT NULL,
	[GameMoneyChangeCode] [tinyint] NOT NULL,
	[GameMoneyAmount] [bigint] NOT NULL,
	[GameMoneyChangeCount] [int] NOT NULL,
 CONSTRAINT [PK_ST_DailyGameMoney] PRIMARY KEY CLUSTERED 
(
	[BasisDate] ASC,
	[WorldID] ASC,
	[GameMoneyChangeCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
