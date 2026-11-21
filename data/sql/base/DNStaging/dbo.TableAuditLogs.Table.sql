USE [dnstaging]
GO
/****** Object:  Table [dbo].[TableAuditLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TableAuditLogs](
	[tableAuditLogSN] [bigint] IDENTITY(1,1) NOT NULL,
	[tableName] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[operationCode] [tinyint] NOT NULL,
	[columnNames] [nvarchar](max) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[beforeImages] [nvarchar](max) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[modifier] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[occurredDate] [datetime] NOT NULL,
 CONSTRAINT [PK_TableAuditLogs] PRIMARY KEY CLUSTERED 
(
	[tableAuditLogSN] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
