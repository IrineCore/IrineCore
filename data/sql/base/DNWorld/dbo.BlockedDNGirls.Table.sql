USE [dnworld]
GO
/****** Object:  Table [dbo].[BlockedDNGirls]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[BlockedDNGirls](
	[DNGirlCharacterID] [bigint] NOT NULL,
	[BlockedDate] [datetime] NOT NULL,
	[BlockedReason] [nvarchar](100) COLLATE Chinese_PRC_CI_AS NULL,
 CONSTRAINT [PK_BlockedDNGirls] PRIMARY KEY CLUSTERED 
(
	[DNGirlCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[BlockedDNGirls] ADD  CONSTRAINT [DF_BlockedDNGirls_BlockedDate]  DEFAULT (getdate()) FOR [BlockedDate]
GO
